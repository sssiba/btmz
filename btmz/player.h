#pragma once

#include "btmz.h"

#include "item.h"

#include "enemy.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#define MAX_PLITEM 16 //プレイヤーの持てるアイテム数
enum { //装備可能箇所
  EQ_HEAD,      //頭
  EQ_WEAPON,    //武器
  EQ_SHIELD,    //盾
  EQ_BODY,      //体
  EQ_HAND,      //手
  EQ_FOOT,      //靴
  EQ_AMULET,    //首飾り
  EQ_RING,      //指輪
  EQMAX
};

/*
 * プレイヤーステータス
 */
typedef struct PLSTAT {
  int16_t hpmax;
  int16_t hp;
  uint8_t lvl;
  int16_t str;
  int16_t dex;
  int16_t intl;
  int16_t luk;
  int16_t ex;
  int16_t mp;
  int16_t mpmax;
  uint32_t gold;

  int16_t minatk;
  int16_t maxatk;
  int16_t def;
  int16_t healhp; //体力回復量(FIX)

  ITEM* equip[EQMAX];
  ITEM* items[MAX_PLITEM];
};


extern int16_t g_plx, g_ply; //固定小数点
extern Rect8 g_plmvrect;
extern Rect8 g_platrect;
extern Rect8 g_pldfrect;
extern PLSTAT g_plstat;
extern uint8_t g_plfloor;


extern void plInit();
extern void plUpdate();
extern void plDraw();
extern void plFinish();



extern void plSetPos( int16_t x, int16_t y );
extern bool plCheckMvRect( int16_t x, int16_t y, Rect8& rect, bool flip );
extern bool plCheckDfRect( int16_t x, int16_t y, Rect8& rect, bool flip );


inline int16_t plGetX() { return TOINT( g_plx ); }
inline int16_t plGetY() { return TOINT( g_ply ); }
inline int16_t plGetFixX() { return g_plx; }
inline int16_t plGetFixY() { return g_ply; }
inline Rect8& plGetMvRect() { return g_plmvrect; }
inline Rect8& plGetAtRect() { return g_platrect; }
inline Rect8& plGetDfRect() { return g_pldfrect; }
inline PLSTAT& plGetStat() { return g_plstat; }
inline uint8_t plGetFloor() { return g_plfloor; }
inline void plSetFloor( uint8_t f ) { g_plfloor = f; }


extern int16_t plCalcDamage( EnemyData* ed );
extern bool plDamage( EnemyData* ed, int16_t dmg );


/*
 * 装備中の武器を返す
 */
extern ITEM* plGetEqWeapon();

/*
 * アイテムを持たせる
 */
extern bool plAddItem( ITEM* item );

/*
 * アイテムを捨てる
 * リストからの削除のみでアイテム自体は破棄しない。
 * リストに隙間が出来ない様に削除箇所以降を前に詰めるのもやる。
 */
extern bool plDelItem( ITEM* item );

/*
 * アイテムの数を返す
 */
extern int8_t plGetItemCount();
inline bool plIsItemFull() {
  return plGetItemCount() == MAX_PLITEM;
}
inline bool plIsItemEmpty() {
  return plGetItemCount() == 0;
}

/*
 * アイテムを上書き
 * olditem の場所に newitem をセット
 */
extern void plOverwriteItem( ITEM* olditem, ITEM* newitem );
 

/*
 * アイテムを装備する
 */
extern bool plEquip( ITEM* item );

/*
 * 装備を外す
 */
extern ITEM* plUnequip( int8_t eqpos );
extern ITEM* plUnequip( ITEM* item );

/*
 * 指定アイテムの装備位置を返す
 */
extern int8_t plCheckEquipPos( ITEM* item ); //指定アイテムの装備箇所を返す

/*
 * 指定箇所に装備しているアイテムを返す
 */
extern ITEM* plGetEquipItem( int8_t eqpos );

#if defined( DBG_MAP )
extern uint8_t DBGtoarea, DBGtoblock;
#endif

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

