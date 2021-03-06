#pragma once

#include <Gamebuino-Meta.h>

#include "gamemain.h"

#include "pic.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#define MAX_ENEMYNAME 10
#define MAX_ENEMYENTRY 40
  
enum {
  ENPHASE_WAIT,
  ENPHASE_MOVE,
  ENPHASE_ATTACK,   //攻撃
  ENPHASE_COOLDOWN, //攻撃後待機
  
};


/*
   敵データ
   テンプレートを元に生成される
*/
typedef struct EnemyData {
  uint8_t type; //type
//  EnemyTemplate* base; //pic, name はテンプレートから引く。
  uint8_t area; //存在する areaID
//  uint8_t block; //自分の属する area 内ブロック
  uint8_t lvl;
  int16_t hpmax;
  int16_t hp;
  uint16_t str; //攻撃力
  uint16_t def; //防御力
  uint8_t stat;
  uint8_t flip;
  uint8_t anm;
  uint8_t anmwait;
  int16_t x, y; //位置(マップ座標) 固定小数点
  uint8_t phase;
  uint8_t phasewait; //特定の phase で使う wait
  uint8_t atkinterval; //攻撃間隔(0の時にのみ攻撃出来る)
  uint8_t wk[2]; //適当に使うワーク
};


/*
   敵データテンプレート
*/
typedef void (*enFunc)(EnemyData*);
typedef struct EnemyTemplate {
  enFunc fInit;
  enFunc fUpdate;
  enFunc fDraw;
  enFunc fFinish;
  uint8_t name[MAX_ENEMYNAME];
  uint8_t lvl;
  uint16_t hpmax;
  uint16_t str;
  uint16_t def;
  uint8_t atkspd; //攻撃間隔(frm)。小さい程速い
  uint8_t w; //配置時の横幅
  uint16_t getexp; //貰える経験値  
  Rect8 mvrect;
  Rect8 atrect;
  Rect8 dfrect;
};




//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
enum ENTYPE : uint8_t {
  ENTYPE_UNDEFINED,
  ENTYPE_SLIME,
  ENTYPE_GHOST,
  ENTYPE_ORC,
  ENTYPEMAX
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
extern void enInit();
extern void enUpdate();
extern void enDraw();
extern void enFinish();
extern int16_t enGetPlayerDist( EnemyData* ed ); //プレイヤーとのX軸上の距離を返す

/* 
 *  敵を生成
 *  ent : 生成する敵の種類
 *  scl : 強さスケール
 *  return EnemyData*
 */
extern EnemyData* enCreate( ENTYPE ent, uint8_t scl, uint8_t area );
inline void enDelete( EnemyData* ed ) {
  ed->type = ENTYPE_UNDEFINED;
}

inline void enSetPos( EnemyData* ed, uint16_t x, uint16_t y ) {
  ed->x = x;
  ed->y = y;
}

/*
 * プレイヤーの攻撃で受けるダメージを計算する
 * x!x! ダメージの種類を設定した方がいいかもね
 */
 extern int16_t enCalcDamage( EnemyData* ed );


/*
 * 指定の値のダメージを受ける
 * ダメージ値自体は別に計算。ここでは必ず指定の値ダメージを受ける。
 * 
 * return true:死んだ  false:生きてる
 */
extern bool enDamage( EnemyData* ed, int16_t dmg);

/*
 * 指定された領域と移動当たり判定が衝突する敵を返す
 * 
 * 一番最初に見つけたものを返す
 */
extern EnemyData* enCheckMvRect( int16_t x, int16_t y, Rect8& rect, bool flip );
extern EnemyData* enCheckDfRect( int16_t x, int16_t y, Rect8& rect, bool flip );

/*
 * 指定の位置から指定のX軸上の範囲内にいる敵の EnemyData* を返す
 */
extern EnemyData* enGetInRange( int16_t x, int16_t y, int16_t range );

extern void enSave( File& f);
extern bool enLoad( File& f);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
extern EnemyTemplate g_enTemplate[ ENTYPEMAX ];

extern EnemyTemplate edSlime;

#define ENTPL( _ID_ ) (&g_enTemplate[ _ID_ ])



