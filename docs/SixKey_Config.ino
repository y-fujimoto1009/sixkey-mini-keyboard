#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Keyboard.h>
#include <SPI.h>
#include <EEPROM.h>
#include <USB.h>
#include <tusb.h>

// Build a complete chord before sending it. Sending ASCII first and Ctrl
// second produces a plain letter; a busy USB endpoint can also drop Ctrl.
class ChordKeyboard : public Keyboard_ {
  KeyReport pendingReport={};
  bool pendingReportDirty=false;
protected:
  void sendReport(KeyReport *report) override {
    pendingReport=*report;
    pendingReportDirty=true;
  }
public:
  bool flushReport() {
    if(!pendingReportDirty)return true;
    if(!_running)return false;
    CoreMutex lock(&USB.mutex);
    tud_task();
    if(USB.HIDReady() && tud_hid_keyboard_report(USB.findHIDReportID(_id),pendingReport.modifiers,pendingReport.keys))pendingReportDirty=false;
    tud_task();
    return !pendingReportDirty;
  }
  bool waitReport() {
    uint32_t started=millis();
    while(!flushReport()) {
      if(millis()-started>=40)return false;
      delay(1);
    }
    return true;
  }
};
ChordKeyboard chordKeyboard;

// RP2040-Zero: preserve the pin assignments of the working hardware.
constexpr uint8_t KEY_PINS[7]={26,27,28,13,14,15,12};
constexpr uint8_t LCD_SCK=2,LCD_MOSI=3,LCD_DC=4,LCD_CS=5,LCD_RST=6,LCD_BLK=7;
constexpr uint8_t ENC_A=10,ENC_B=11;
constexpr uint16_t DEFAULTS[9]={49,50,51,52,53,54,2001,2002,2003};
constexpr uint32_t MAGIC=0x364b0001;
struct Config{uint32_t magic;uint16_t map[9];uint16_t checksum;};
struct Button{bool raw,down;uint32_t changed;};
Config config;
Button buttons[7];
bool sent[256]={},armed=false;
uint32_t releasedAt=0;
uint8_t encPrevious;
int8_t quarters=0;
char line[120];size_t used=0;bool overflow=false;
Adafruit_ST7735 tft(&SPI,LCD_CS,LCD_DC,LCD_RST);

bool validAction(uint16_t a){return a==0||a==32||(a>=48&&a<=57)||(a>=97&&a<=122)||(a>=128&&a<=131)||(a>=176&&a<=179)||(a>=194&&a<=205)||(a>=212&&a<=212)||(a>=215&&a<=218)||(a>=1001&&a<=1006)||(a>=1101&&a<=1106)||(a>=1201&&a<=1202)||(a>=2001&&a<=2006);}
uint16_t checksum(const Config &c){uint16_t h=0x6b31;for(auto a:c.map)h=(h*31)^a;return h;}
bool validConfig(const Config &c){if(c.magic!=MAGIC||c.checksum!=checksum(c))return false;for(auto a:c.map)if(!validAction(a))return false;return true;}
void releaseHid(){chordKeyboard.releaseAll();chordKeyboard.waitReport();chordKeyboard.consumerRelease();memset(sent,0,sizeof(sent));}
void draw(){tft.fillScreen(ST77XX_BLACK);tft.setTextSize(1);tft.setTextColor(ST77XX_WHITE);tft.setCursor(3,3);tft.print("6KEY CONFIG V1.2");for(int i=0;i<6;i++){int x=(i%3)*53,y=18+(i<3?23:0);tft.fillRoundRect(x+1,y,50,20,3,buttons[i].down?ST77XX_GREEN:ST77XX_BLUE);tft.setCursor(x+4,y+6);tft.print(i+1);tft.print(':');tft.print(config.map[i]);}tft.setCursor(3,68);tft.print(armed?"READY":"Release all keys");}
void report(){Serial.print("K6/1 MAP ");for(int i=0;i<9;i++){if(i)Serial.print(',');Serial.print(config.map[i]);}Serial.println();}
void command(){
  line[used]=0;
  if(!strcmp(line,"K6/1 GET")){report();return;}
  if(strncmp(line,"K6/1 SET ",9)){Serial.println("K6/1 ERR COMMAND");return;}
  Config next={};next.magic=MAGIC;const char *p=line+9;
  for(int i=0;i<9;i++){
    if(*p<'0'||*p>'9'){Serial.println("K6/1 ERR FORMAT");return;}
    unsigned value=0;while(*p>='0'&&*p<='9'){value=value*10+(*p++-'0');if(value>3000){Serial.println("K6/1 ERR RANGE");return;}}
    if(!validAction(value)||(*p!=(i==8?'\0':','))){Serial.println("K6/1 ERR VALUE");return;}
    next.map[i]=value;if(i<8)p++;
  }
  next.checksum=checksum(next);releaseHid();armed=false;releasedAt=0;quarters=0;
  EEPROM.put(0,next);if(!EEPROM.commit()){Serial.println("K6/1 ERR FLASH");return;}
  // begin() re-reads physical flash rather than merely checking the RAM buffer.
  EEPROM.begin(256);Config saved={};EEPROM.get(0,saved);
  if(!validConfig(saved)||memcmp(&saved,&next,sizeof(next))){Serial.println("K6/1 ERR VERIFY");return;}
  config=saved;draw();report();
}
void serialPoll(){int budget=128;while(Serial.available()&&budget--){char c=Serial.read();if(c=='\r')continue;if(c=='\n'){if(overflow)Serial.println("K6/1 ERR LENGTH");else command();used=0;overflow=false;}else if(used<sizeof(line)-1&&!overflow)line[used++]=c;else overflow=true;}}
void addAction(bool *desired,uint16_t a){if(a>0&&a<256)desired[a]=true;else if(a>=1001&&a<=1006){const char keys[]="cvxzsa";desired[KEY_LEFT_CTRL]=true;desired[(uint8_t)keys[a-1001]]=true;}else if(a>=1101&&a<=1106){const char keys[]="cvxzsa";desired[KEY_LEFT_GUI]=true;desired[(uint8_t)keys[a-1101]]=true;}else if(a>=1201&&a<=1202){const char keys[]="av";desired[KEY_LEFT_ALT]=true;desired[(uint8_t)keys[a-1201]]=true;}}
void mediaTap(uint16_t a){if(a<2001||a>2006)return;const uint16_t codes[]={0xe9,0xea,0xe2,0xcd,0xb5,0xb6};chordKeyboard.consumerPress(codes[a-2001]);delay(12);chordKeyboard.consumerRelease();}
void syncHeld(){bool desired[256]={};for(int i=0;i<7;i++)if(buttons[i].down)addAction(desired,config.map[i==6?8:i]);for(int i=1;i<256;i++)if(sent[i]&&!desired[i])chordKeyboard.release(i);for(int i=1;i<256;i++)if(!sent[i]&&desired[i])chordKeyboard.press(i);memcpy(sent,desired,sizeof(sent));chordKeyboard.flushReport();}
void tap(uint16_t a){if(a>=2001){mediaTap(a);return;}bool desired[256]={};addAction(desired,a);for(int i=1;i<256;i++)if(desired[i]&&!sent[i])chordKeyboard.press(i);if(chordKeyboard.waitReport())delay(12);for(int i=1;i<256;i++)if(desired[i]&&!sent[i])chordKeyboard.release(i);chordKeyboard.waitReport();}
void setup(){
  pinMode(LCD_BLK,OUTPUT);digitalWrite(LCD_BLK,LOW);
  for(int i=0;i<7;i++){pinMode(KEY_PINS[i],INPUT_PULLUP);buttons[i]={digitalRead(KEY_PINS[i])==LOW,digitalRead(KEY_PINS[i])==LOW,millis()};}
  pinMode(ENC_A,INPUT_PULLUP);pinMode(ENC_B,INPUT_PULLUP);encPrevious=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
  EEPROM.begin(256);EEPROM.get(0,config);if(!validConfig(config)){config={};config.magic=MAGIC;memcpy(config.map,DEFAULTS,sizeof(DEFAULTS));config.checksum=checksum(config);}
  Serial.begin(115200);chordKeyboard.begin();releaseHid();
  SPI.setSCK(LCD_SCK);SPI.setTX(LCD_MOSI);SPI.begin();tft.initR(INITR_MINI160x80);tft.setRotation(3);draw();digitalWrite(LCD_BLK,HIGH);
}
void loop(){
  serialPoll();uint32_t now=millis();bool released=true,changed=false;
  for(int i=0;i<7;i++){auto &b=buttons[i];bool down=digitalRead(KEY_PINS[i])==LOW;if(down!=b.raw){b.raw=down;b.changed=now;}if(b.down!=b.raw&&now-b.changed>=8){b.down=b.raw;changed=true;if(armed&&b.down)mediaTap(config.map[i==6?8:i]);}released&=!b.down&&!b.raw;}
  if(!armed){if(released){if(!releasedAt)releasedAt=now;if(now-releasedAt>=500){armed=true;changed=true;}}else releasedAt=0;}
  if(armed)syncHeld();else chordKeyboard.flushReport();if(changed)draw();
  uint8_t current=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
  static const int8_t transitions[16]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  if(current!=encPrevious){quarters+=transitions[(encPrevious<<2)|current];encPrevious=current;if(!armed)quarters=0;else if(quarters>=4||quarters<=-4){tap(config.map[quarters>0?6:7]);quarters=0;}}
}
