#pragma once

#include <Gamebuino-Meta.h>

#include "btmz.h"

//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
enum PICID : uint8_t {
  PIC_DUNGEONBG,
  PIC_CHARACTER,
  PIC_WIZARD,
  PIC_EFFECT8x8,
  PIC_OBJECT, //8x8
  PIC_OBJECT8x16, //8x16
  PIC_OBJECT16x8, //16x8
  PIC_OBJECT12x16, //12x16
  PIC_ICON6x5, //icon
  PIC_TITLE, //title
  PIC_TITLEFIRE, //タイトルのろうそくの火。わざわざここに入れなくても良い感じ


  PICINVALID = 0xff
};

extern Image* g_pictbl[];
inline Image* getPic( PICID pic )
{
  return g_pictbl[pic];
}


//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
typedef struct ItemPic {
  Image* pic;
  uint8_t frm;
  int8_t x, y, w, h;  
};

//WEAPON
extern ItemPic ipShortSword;   //落ちてる時
extern ItemPic ipEqShortSword; //装備時
extern ItemPic ipLongSword;
extern ItemPic ipEqLongSword;
extern ItemPic ipDagger;
extern ItemPic ipEqDagger;
extern ItemPic ipBlade;
extern ItemPic ipEqBlade;
extern ItemPic ipBroadSword;
extern ItemPic ipEqBroadSword;
extern ItemPic ipCutlass;
extern ItemPic ipEqCutlass;
extern ItemPic ipMace;
extern ItemPic ipEqMace;
extern ItemPic ipWarMace;
extern ItemPic ipEqWarMace;
extern ItemPic ipKatana;
extern ItemPic ipEqKatana;
extern ItemPic ipRuneSword;
extern ItemPic ipEqRuneSword;
extern ItemPic ipLordSword;
extern ItemPic ipEqLordSword;


//SHIELD
extern ItemPic ipWoodShield;
extern ItemPic ipRoundShield;

//HEAD
extern ItemPic ipLeatherHelm;
extern ItemPic ipHelm;

//ARMOR
extern ItemPic ipLeatherArmor;
extern ItemPic ipChainMail;

//FEET
extern ItemPic ipLeatherBoots;
extern ItemPic ipShoes;

//HAND
extern ItemPic ipLeatherGlove;
extern ItemPic ipChainGlove;

//AMULET
extern ItemPic ipStoneAmulet;
extern ItemPic ipAmulet;

//RING
extern ItemPic ipWoodRing;
extern ItemPic ipIronRing;

//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------

typedef struct EnemyPic {
  Image* img;
  uint8_t  w, h;
  uint8_t  wait;    //待機状態
  uint8_t  move[2]; //移動
  uint8_t  attack;  //攻撃
};

extern EnemyPic epSlime;
extern EnemyPic epGhost;
extern EnemyPic epOrc;



//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------

