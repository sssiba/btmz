/*
 * 特殊攻撃
 * magic, poison, ...
 * プレイヤーも敵も共通
 */
#pragma once

#include "btmz.h"


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
enum SpAtkType : uint8_t {
  SPATK_NONE,
  SPATK_FIREBALL, //火の玉
  SPATK_ICEBALL,
  SPATK_THUNDER,  //サンダー
  SPATK_FLAME,    //炎
  SPATK_FIREBRESS, //炎の息
  SPATK_ICEBRESS,  //氷の息
  SPATK_SLEEP,    //眠り
};

#define MAX_SPATK 24 //特殊攻撃最大数


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
class SpAtCtrl
{


public:
  enum {
    FLAG_TGT_ENEMY = (1<<0), //敵が対象となる
    FLAG_TGT_PLAYER = (1<<1), //プレイヤーが対象となる
    FLAG_FLIP = (1<<2), //反転
    FLAG_DONEHIT = (1<<3), //当たり判定完了
  };

  struct SPADATA {
    SpAtkType type;
    uint8_t flag;
    int16_t x;
    int16_t y;
    int16_t fx, fy;
    int16_t value; //効果値（攻撃ならダメージ。回復なら回復量等)
    uint8_t duration; //持続時間
//    SpAtkUpdateFunc funcUpdate;
//    SpAtkDrawFunc funcDraw;
  };

  typedef void (*SpAtkUpdateFunc)(SPADATA*);
  typedef void (*SpAtkDrawFunc)(SPADATA*);

public:
  SpAtCtrl();
  ~SpAtCtrl();

  void update();
  void draw();

  bool createFireBall( uint8_t tgtflag, int16_t sx, int16_t sy, int16_t tx, int16_t ty, int16_t dmg );

private:
  SPADATA* getEmptyData();
  void updateFIREBALL( SPADATA* spad );
  void drawFIREBALL( SPADATA* spad );

  inline void clrData( SPADATA* spad ) { spad->type = SPATK_NONE; }

  void saDamage( SPADATA* spad );

private:
  SPADATA m_spadata[ MAX_SPATK ];

};


extern SpAtCtrl g_spatctrl;

#define SPAC() g_spatctrl

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

