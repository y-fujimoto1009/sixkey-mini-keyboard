#pragma once
#include <Adafruit_GFX.h>

// All three games run on the device. Coordinates are for a 160x80 LCD.
namespace TinyGames {
enum Scene { MENU, JUMP, CATCH, MEMORY, KEYS };
Scene scene=MENU;
uint8_t selected=0,score=0,lives=3;
bool ended=false,won=false;
uint32_t rng=0x13579bdf, ticks=0;
int bunnyY=43,velocity=0,obstacleX=176,catX=80;
struct Drop { int x,y; bool active,berry; };
Drop drops[4];
uint8_t sequence[8],roundSize=1,inputPosition=0,showPosition=0;
bool showing=true,lit=false;
uint32_t phaseAt=0;
uint16_t nextRandom(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return uint16_t(rng);}
void start(Scene s,uint32_t now){
 scene=s;score=0;lives=3;ended=false;won=false;ticks=0;
 bunnyY=43;velocity=0;obstacleX=176;catX=80;
 for(auto &d:drops)d={0,0,false,false};
 roundSize=1;inputPosition=0;showPosition=0;showing=true;lit=false;phaseAt=now;
 for(auto &v:sequence)v=nextRandom()%3;
}
void finish(bool win){ended=true;won=win;}
void step(const bool *held,const bool *pressed,int turn,uint32_t now){
 ticks++;
 if(scene==MENU){selected=(selected+4+turn%4)%4;if(pressed[0])selected=(selected+3)%4;if(pressed[2])selected=(selected+1)%4;return;}
 if(ended||scene==KEYS)return;
 if(scene==JUMP){
  if((pressed[0]||pressed[1]||pressed[2])&&bunnyY==43)velocity=-7;
  if(bunnyY<43||velocity){bunnyY+=velocity;velocity++;if(bunnyY>=43){bunnyY=43;velocity=0;}}
  obstacleX-=2+score/8;
  if(obstacleX< -12){obstacleX=176+nextRandom()%65;if(++score>=20)finish(true);}
  if(obstacleX<41&&obstacleX+10>31&&bunnyY+18>51){
   obstacleX=176+nextRandom()%65;if(--lives==0)finish(false);
  }
 }else if(scene==CATCH){
  catX+=(held[2]?3:0)-(held[0]?3:0);catX=constrain(catX,12,148);
  if(ticks%32==1)for(auto &d:drops)if(!d.active){d={12+int(nextRandom()%137),14,true,nextRandom()%5!=0};break;}
  for(auto &d:drops)if(d.active){
   d.y+=1+(score>=12&&ticks%2==0?1:0);
   if(d.y>=55&&d.y<68&&abs(d.x-catX)<=14){d.active=false;if(d.berry){if(++score>=20)finish(true);}else if(--lives==0)finish(false);}
   else if(d.y>=72){d.active=false;if(d.berry&&--lives==0)finish(false);}
   if(ended)break;
  }
 }else if(scene==MEMORY){
  if(showing){
   if(uint32_t(now-phaseAt)>=(lit?520u:280u)){
    phaseAt=now;
    if(lit){lit=false;if(++showPosition>=roundSize){showing=false;inputPosition=0;}}
    else lit=true;
   }
  }else{
   for(int i=0;i<3;i++)if(pressed[i]){
    if(i!=sequence[inputPosition]){finish(false);break;}
    if(++inputPosition>=roundSize){score=roundSize;if(roundSize==8){finish(true);break;}roundSize++;showPosition=0;showing=true;lit=false;phaseAt=now;break;}
   }
  }
 }
}
constexpr uint16_t INK=0x51ca,CREAM=0xff9b,PINK=0xfbd5,MINT=0x9f35,SKY=0x9e7f,GREEN=0x5d4c;
void text(Adafruit_GFX &p,int x,int y,const char *s,uint16_t c=INK){p.setTextColor(c);p.setTextSize(1);p.setCursor(x,y);p.print(s);}
void sprite(Adafruit_GFX &p,int x,int y,const char *const *rows,int count,uint16_t body){
 for(int j=0;j<count;j++)for(int i=0;rows[j][i];i++){char c=rows[j][i];if(c!=' ')p.drawPixel(x+i,y+j,c=='#'?INK:c=='p'?PINK:c=='w'?0xffff:body);}
}
void bunny(Adafruit_GFX &p,int x,int y){
 static const char *const art[]={"   ww   ww      ","   wpw  wpw     ","   wpw  wpw     ","   www  www     ","   wwwwwwww     ","  wwwwwwwwww    "," wwwwwwwwwwww   "," ww#wwwwww#ww   "," wppwwwwwwppw   "," wwwww#wwwwww   ","  wwwwwwwwww    ","   wwwwwwww     ","   wwwwwwww  ww ","  wwwwwwwwwwwww ","  wwwwwwwwwwww  ","  wwwwwwwwwwww  ","  wwwwwwwwwwww  ","   www   www    "};sprite(p,x,y,art,18,0xffff);
}
void cat(Adafruit_GFX &p,int x,int y){
 static const char *const art[]={" b          b "," bbb      bbb "," bppbbbbbbppb "," bbbbbbbbbbbb "," bbbbbbbbbbbb "," bb#bbbbbb#bb "," bppbbb#bbppb "," bbbb#####bbb ","  bbbbbbbbbb  ","   bbbbbbbb   ","  bbbbbbbbbb  "," bbbbbbbbbbbb "};sprite(p,x,y,art,12,0xfed5);
}
void flower(Adafruit_GFX &p,int x,int y,uint16_t color,bool glow){
 if(glow)p.fillRoundRect(x-19,y-19,38,38,10,0xffff);
 p.fillCircle(x,y-7,6,color);p.fillCircle(x-7,y,6,color);p.fillCircle(x+7,y,6,color);p.fillCircle(x-4,y+7,6,color);p.fillCircle(x+4,y+7,6,color);p.fillCircle(x,y,5,0xff6e);
 p.drawPixel(x-2,y,INK);p.drawPixel(x+2,y,INK);p.drawFastHLine(x-1,y+3,3,INK);
}
void draw(Adafruit_GFX &p){
 if(scene==KEYS)return;
 p.fillScreen(CREAM);
 if(scene==MENU){
  text(p,5,4,"POCKET PLAY");text(p,5,16,"K1 <   K2 OK   > K3");
  const char *names[]={"BUNNY HOP","SNACK CATCH","FLOWER MEMORY","KEYBOARD"};
  p.fillRoundRect(4,30,152,29,8,selected==0?PINK:selected==1?MINT:selected==2?SKY:0xe71c);
  if(selected==0)bunny(p,12,35);else if(selected==1)cat(p,12,38);else if(selected==2)flower(p,21,44,PINK,false);
  text(p,selected==3?12:42,42,names[selected]);text(p,5,68,"K6: keyboard");return;
 }
 p.fillRect(0,0,160,13,0xffff);text(p,3,3,scene==JUMP?"HOP":scene==CATCH?"CATCH":"MEMORY");
 p.setCursor(55,3);p.print(scene==MEMORY?roundSize:score);p.print(scene==MEMORY?"/8":"/20");
 if(scene!=MEMORY){p.setCursor(113,3);p.print("<3 ");p.print(lives);}
 if(scene==JUMP){
  p.fillCircle(128,28,10,0xff6e);p.fillRoundRect(70,23,27,6,3,0xffff);p.fillRoundRect(11,26,17,5,3,0xffff);
  p.fillRect(0,61,160,19,MINT);p.drawFastHLine(0,61,160,GREEN);bunny(p,28,bunnyY);
  p.fillRect(obstacleX+4,54,4,7,0xff3b);p.fillRoundRect(obstacleX,50,12,6,3,PINK);p.drawPixel(obstacleX+3,52,0xffff);p.drawPixel(obstacleX+8,52,0xffff);
  text(p,5,70,"K1 / K2 / K3 : jump");
 }else if(scene==CATCH){
  p.fillRect(0,13,160,52,SKY);p.fillRoundRect(18,23,25,5,3,0xffff);p.fillRoundRect(117,29,27,5,3,0xffff);
  for(auto &d:drops)if(d.active){if(d.berry){p.fillCircle(d.x,d.y,4,PINK);p.drawFastHLine(d.x-2,d.y-5,5,GREEN);p.drawPixel(d.x-1,d.y,0xffff);}else{p.fillCircle(d.x,d.y,4,INK);p.drawPixel(d.x-1,d.y-1,0xffff);}}
  cat(p,catX-7,52);p.fillRoundRect(catX-14,63,28,5,2,0xe4cc);text(p,5,71,"K1 <      > K3");
 }else{
  uint16_t colors[]={PINK,MINT,SKY};for(int i=0;i<3;i++)flower(p,27+i*53,41,colors[i],showing&&lit&&sequence[showPosition]==i);
  text(p,22,61,"K1       K2      K3");text(p,36,72,showing?"WATCH...":"YOUR TURN!");
 }
 if(ended){
  p.fillRoundRect(12,20,136,43,7,0xffff);p.drawRoundRect(12,20,136,43,7,PINK);
  text(p,won?47:38,29,won?"YOU DID IT!":"NICE TRY <3");text(p,23,47,"K2: try again");
 }
}
}
