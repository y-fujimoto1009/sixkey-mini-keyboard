#pragma once
#include <stdint.h>
inline int constrain(int v,int lo,int hi){return v<lo?lo:v>hi?hi:v;}
inline int abs(int v){return v<0?-v:v;}
// Headless substitute: game logic is real; LCD transport/rendering are not emulated.
class Adafruit_GFX {
public:
 void setTextColor(uint16_t){} void setTextSize(int){} void setCursor(int,int){}
 template<class T> void print(T){}
 void drawPixel(int,int,uint16_t){} void fillRect(int,int,int,int,uint16_t){}
 void fillRoundRect(int,int,int,int,int,uint16_t){} void drawRoundRect(int,int,int,int,int,uint16_t){}
 void fillCircle(int,int,int,uint16_t){} void drawFastHLine(int,int,int,uint16_t){}
 void fillScreen(uint16_t){}
};
