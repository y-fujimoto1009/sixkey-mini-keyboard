#include "../firmware/SixKey_Play/Games.h"
using namespace TinyGames;
volatile uint32_t testFailure=0,testsPassed=0;
#define CHECK(expr,id) if(!(expr)){testFailure=id;return id;}
extern "C" int run_game_tests(){
 bool held[7]={},press[7]={};uint32_t now=1000;
 start(MENU,now);selected=0;press[0]=true;step(held,press,0,now);CHECK(selected==3,1);
 press[0]=false;press[2]=true;step(held,press,0,now);CHECK(selected==0,2);press[2]=false;testsPassed++;
 start(JUMP,now);
 for(int i=0;i<2000&&!ended;i++)step(held,press,0,now+=25);
 CHECK(ended&&!won&&lives==0,3);testsPassed++;
 start(JUMP,now);
 for(int i=0;i<4000&&!ended;i++){
  press[0]=bunnyY==43&&obstacleX<=48&&obstacleX>=40;
  step(held,press,0,now+=25);CHECK(bunnyY<=43&&bunnyY>=0,4);
 }
 CHECK(ended&&won&&score==20&&lives>0,5);testsPassed++;press[0]=false;
 start(CATCH,now);held[0]=true;for(int i=0;i<100;i++)step(held,press,0,now+=25);CHECK(catX==12,6);
 held[0]=false;held[2]=true;for(int i=0;i<100;i++)step(held,press,0,now+=25);CHECK(catX==148,7);held[2]=false;testsPassed++;
 start(CATCH,now);for(auto &d:drops)d={80,54,true,false};step(held,press,0,now+=25);
 CHECK(ended&&!won&&lives==0,8);testsPassed++;
 start(CATCH,now);score=19;drops[0]={80,54,true,true};step(held,press,0,now+=25);CHECK(ended&&won&&score==20,9);testsPassed++;
 start(MEMORY,now);press[0]=true;step(held,press,0,now+=25);CHECK(inputPosition==0&&!ended,10);press[0]=false;testsPassed++;
 for(int round=1;round<=8;round++){
  int guard=0;while(showing&&guard++<400)step(held,press,0,now+=25);
  CHECK(!showing&&roundSize==round,11);
  for(int i=0;i<round;i++){int choice=sequence[i];press[choice]=true;step(held,press,0,now+=25);press[choice]=false;}
 }
 CHECK(ended&&won&&score==8,12);testsPassed++;
 start(MEMORY,now);while(showing)step(held,press,0,now+=25);
 press[(sequence[0]+1)%3]=true;step(held,press,0,now+=25);CHECK(ended&&!won,13);testsPassed++;
 Adafruit_GFX painter;for(int sceneId=0;sceneId<=4;sceneId++){scene=static_cast<Scene>(sceneId);draw(painter);}testsPassed++;
 return 0;
}
