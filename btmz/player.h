#pragma once

#include "btmz.h"

#include "item.h"

#include "enemy.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
/*
 * アイテムに依る強化値
 */
typedef struct PLITEMBUFF {
  int16_t minatk;
  int16_t maxatk;
  int16_t addhpmax;
  int16_t addmpmax;
  int16_t adddmg; //%
  int16_t addstr;
  int16_t adddex;
  int16_t addint;
  int16_t addluk;
  int16_t def;
  int16_t healhp; //体力回復量(FIX)
  int8_t registfire; //100%
  int8_t registcold; //100%
  int8_t registphysic;
  int8_t registthunder;
  int8_t registmagic;
  ItemAttribute itemattr;
};
/*
 * プレイヤーステータス
 */
typedef struct PLSTAT {
  char name[13];
  uint8_t lvl;
  int16_t hpmax;
  int16_t hp;
  int16_t str;
  int16_t dex;
  int16_t intl;
  int16_t luk;
  int16_t ex;
  int16_t mp;
  int16_t mpmax;
  uint32_t gold;
  uint8_t curfloor; //現在居るフロア(0:地上)
  PLITEMBUFF itbuff;
  int16_t nextexp; //次のレベルへの必要経験値。plCalcExpReqNextLvl() は遅そうなので、１回計算して置いておく。
};

#define MAX_PL_LVL 99
#define MAX_PL_HPMAX 9999
#define MAX_PL_STR 999
#define MAX_PL_DEX 999
#define MAX_PL_INT 999
#define MAX_PL_LUK 999
#define MAX_PL_EXP 29999
#define MAX_PL_MPMAX 999
#define MAX_PL_GOLD 9999999
#define MAX_PL_MINATK 9999
#define MAX_PL_MAXATK 9999
#define MAX_PL_ADDHPMAX MAX_PL_HPMAX
#define MAX_PL_ADDSTR MAX_PL_STR
#define MAX_PL_ADDDEX MAX_PL_DEX
#define MAX_PL_ADDINT MAX_PL_INT
#define MAX_PL_ADDLUK MAX_PL_LUK
#define MAX_PL_DEF 9999
#define MAX_PL_HEALHP 999
#define MAX_PL_ADDDMG 9999
#define MAX_PL_RGFIRE 100
#define MAX_PL_RGCOLD 100
#define MAX_PL_RGPHYSIC 100
#define MAX_PL_RGTHUNDER 100
#define MAX_PL_RGMAGIC 100





extern ITEM* g_plequip[EQMAX];
extern ITEM* g_plitems[MAX_PLITEM];

extern int16_t g_plx, g_ply; //固定小数点
extern Rect8 g_plmvrect;
extern Rect8 g_platrect;
extern Rect8 g_pldfrect;
extern PLSTAT g_plstat;

extern int16_t g_wzx, g_wzy;

extern void plInit();
extern void plUpdate();
extern void plDraw();
extern void plFinish();



extern void plSetPos( int16_t x, int16_t y );
extern void wzSetPos( int16_t x, int16_t y );
extern bool plCheckMvRect( int16_t x, int16_t y, Rect8& rect, bool flip );
extern bool plCheckDfRect( int16_t x, int16_t y, Rect8& rect, bool flip );

//player が別フロア or 別エリアに入った際の位置初期化
extern void plSetEnterPos( int16_t x, int16_t y );

inline int16_t plGetX() { return TOINT( g_plx ); }
inline int16_t plGetY() { return TOINT( g_ply ); }
inline int16_t plGetFixX() { return g_plx; }
inline int16_t plGetFixY() { return g_ply; }
inline Rect8& plGetMvRect() { return g_plmvrect; }
inline Rect8& plGetAtRect() { return g_platrect; }
inline Rect8& plGetDfRect() { return g_pldfrect; }
inline PLSTAT& plGetStat() { return g_plstat; }
inline uint8_t plGetFloor() { return g_plstat.curfloor; } //0:地上？ 1-???
inline void plSetFloor( uint8_t f ) { g_plstat.curfloor = f; }

inline int16_t wzGetX() { return TOINT(g_wzx); }
inline int16_t wzGetY() { return TOINT(g_wzy); }

extern int16_t plCalcDamage( EnemyData* ed );
extern bool plDamage( int16_t dmg );

/*
 * 次のレベルへの必要経験値を返す
 * curlvl ... 現在のレベル
 */
extern int16_t plCalcExpReqNextLvl( uint8_t curlvl );

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

/*
 * 経験値取得
 */
extern void plAddExp( uint16_t getexp );

/*
 * 1level up
 */
extern void plLevelUp();

/*
 * 各種ステータス取得
 */
inline char* plGetNAME()
{
  return g_plstat.name;
}
inline int8_t plGetLVL()
{
  return g_plstat.lvl;
}
inline int16_t plGetHP()
{
  return g_plstat.hp;
}
inline int16_t plGetMP()
{
  return g_plstat.mp;
}
inline int16_t plGetEXP()
{
  return g_plstat.ex;
}
inline uint32_t plGetGOLD()
{
  return g_plstat.gold;
}
inline int16_t plGetMINATK()
{
  //アイテムの効果の乱数によって min と max が逆転する事があるかも
  return (g_plstat.itbuff.minatk < g_plstat.itbuff.maxatk) ? g_plstat.itbuff.minatk : g_plstat.itbuff.maxatk;
}
inline int16_t plGetMAXATK()
{
  //アイテムの効果の乱数によって min と max が逆転する事があるかも
  return (g_plstat.itbuff.maxatk > g_plstat.itbuff.minatk) ? g_plstat.itbuff.maxatk : g_plstat.itbuff.minatk;
}
inline int16_t plGetADDDMG()
{
  return g_plstat.itbuff.adddmg;
}
inline int16_t plGetDEF()
{
  return g_plstat.itbuff.def;
}
inline int16_t plGetHEALHP()
{
  return g_plstat.itbuff.healhp;
}
inline int8_t plGetREGISTFIRE()
{
  return g_plstat.itbuff.registfire;
}
inline int8_t plGetREGISTCOLD()
{
  return g_plstat.itbuff.registcold;
}
inline int8_t plGetREGISTPHYSIC()
{
  return g_plstat.itbuff.registphysic;
}
inline int8_t plGetREGISTTHUNDER()
{
  return g_plstat.itbuff.registthunder;
}
inline int8_t plGetREGISTMAGIC()
{
  return g_plstat.itbuff.registmagic;
}
inline ItemAttribute plGetITEMATTR()
{
  return g_plstat.itbuff.itemattr;
}

extern int16_t plGetHPMAX();
extern int16_t plGetMPMAX();
extern int16_t plGetSTR();
extern int16_t plGetDEX();
extern int16_t plGetINT();
extern int16_t plGetLUK();

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
extern void plSave();
extern void plLoad();

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

