#pragma once

#include <Gamebuino-Meta.h>

#include "btmz.h"

//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
enum PICID {
  PIC_DUNGEONBG,
  PIC_CHARACTER,
  PIC_OBJECT, //8x8
  PIC_OBJECT16x8, //16x8
  PIC_OBJECT12x16, //12x16
  PIC_ICON6x5, //icon
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

extern ItemPic ipShortSword;   //落ちてる時
extern ItemPic ipEqShortSword; //装備時
extern ItemPic ipLongSword;
extern ItemPic ipEqLongSword;

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


//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------

