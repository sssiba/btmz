#pragma once

#include "btmz.h"

#include "pic.h"


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

#define MAXITSTR 12

enum ItemType : int8_t {
  IT_UNDEFINED = -1, //未定義
  IT_WEAPON,
  IT_SHIELD,
  IT_HEAD,
  IT_ARMOR,
  IT_FEET,
  IT_HAND,
  IT_RING,
  IT_AMULET,
  IT_POTION,
  IT_SCROLL,
  IT_KEY,
  ITMAX
};

#define ITB( _ITYPE ) ((uint16_t)(1<<IT_##_ITYPE))
#define ITB_ALL() (ITB(WEAPON)|ITB(SHIELD)|ITB(HEAD)|ITB(ARMOR)|ITB(FEET)|ITB(HAND)|ITB(RING)|ITB(AMULET)|ITB(POTION)|ITB(SCROLL)|ITB(KEY)) //全種類
#define ITB_EQALL() (ITB(WEAPON)|ITB(SHIELD)|ITB(HEAD)|ITB(ARMOR)|ITB(FEET)|ITB(HAND)|ITB(RING)|ITB(AMULET)) //装備するもの全て
#define ITB_EQMAIN() (ITB(WEAPON)|ITB(SHIELD)|ITB(HEAD)|ITB(ARMOR)|ITB(FEET)|ITB(HAND)) //AMULET, RING 以外の装備
#define ITB_EQSUB() (ITB(RING)|ITB(AMULET)) //AMULET, RIG
#define ITB_USEALL() (ITB(POTION)|ITB(SCROLL)|ITB(KEY)) //装備するもの以外の種類(使うもの)

enum ItemUseEffect : uint8_t { //使用時効果
  IUE_NOTHING,  //効果無し
  IUE_HEALING,  //HP 回復
  IUE_TELEPORT, //テレポート
  IUEMAX
};

enum ItemAttribute : uint32_t { //特殊効果属性
  IA_FIREDAMAGE = 1 << 0, //炎ダメージ
  IA_COLDDAMAGE = 1 << 1, //冷気ダメージ
  IA_LIGHT = 1 << 2, //明かり
  IA_WARNING = 1 << 3, //同じエリアに敵がいると光る
  IA_HEALING = 1 << 4, //体力回復
};

typedef struct { //基本情報
  ItemType type;
  ItemPic* pic; //落ちてる時の絵
  ItemPic* eqpic; //装備時の絵(現状武器だけ)
  char name[MAXITSTR+1]; //名前
  int16_t minatk;
  int16_t maxatk;
  int16_t def;
  ItemUseEffect useeff; //使用時効果
  uint8_t brokenrate; //使用時破損率(100 で必ず壊れる)
//  uint16_t cprefix; //付加可能性のある prefix (bit)
//  uint16_t csuffix; //付加可能性のある suffix (bit)
} ITEMBASE;


/*
 * ItemName
 * prefix[7] + name[12] + suffix[12]
 * Crimson ShortSword of Lightning
 */
enum ItemPrefix : int8_t { //前置詞
  ITPREFIX_NONE,
  ITPREFIX_SHINY,
  ITPREFIX_LIGHT,
  ITPREFIX_GLOWING,
  ITPREFIX_AGILE,
  ITPREFIX_HARD,
  ITPREFIX_STABLE,
  ITPREFIX_OGRE,
  ITPREFIX_TITAN,
  ITPREFIX_HOLY,
  ITPREFIX_DEVOUT,
  ITPREFIXMAX
};

enum ItemSuffix : int8_t { //後置詞
  ITSUFFIX_NONE,
  ITSUFFIX_MOON,
  ITSUFFIX_STAR,
  ITSUFFIX_SUN,
  ITSUFFIX_POWER,
  ITSUFFIX_FORCE,
  ITSUFFIX_MIND,
  ITSUFFIX_SOUL,
  ITSUFFIX_HEALING,
  ITSUFFIX_RECOVERY,
  ITSUFFIX_KING,
  ITSUFFIX_QUEEN,
  ITSUFFIX_EMPEROR,
  ITSUFFIX_FIRE,
  ITSUFFIX_FLAME,
  ITSUFFIX_COLD,
  ITSUFFIX_ICE,
  ITSUFFIXMAX
};


enum ItemBaseIdx : int16_t {
  IBI_UNDEFINED = -1,
  //IT_WEAPON
  IBI_TOP_WEAPON,
  IBI_DAGGER = IBI_TOP_WEAPON,
  IBI_SHORTSWORD,
  IBI_BLADE,
  IBI_BROARDSWORD,
  IBI_MACE,
  IBI_LONGSWORD,
  IBI_CUTLASS,
  IBI_WARMACE,
  IBI_KATANA,
  IBI_RUNESWORD,
  IBI_LORDSWORD,
  IBI_MAX_WEAPON,

  //IT_SHIELD
  IBI_TOP_SHIELD = IBI_MAX_WEAPON,
  IBI_WOODSHIELD = IBI_TOP_SHIELD,
  IBI_ROUNDSHIELD,
  IBI_BUCKLER,
  IBI_HEATERSHIELD,
  IBI_LARGESHIELD,
  IBI_KITESHIELD,
  IBI_TOWERSHIELD,
  IBI_MAX_SHIELD,

  //IT_HEAD
  IBI_TOP_HEAD = IBI_MAX_SHIELD,
  IBI_LEATHERHELM = IBI_TOP_HEAD,
  IBI_HELM,
  IBI_CLOSEDHELM,
  IBI_GREATHELM,
  IBI_BASINET,
  IBI_COIF,
  IBI_SKULLCAP,
  IBI_CIRCLET,
  IBI_HOOD,
  IBI_MASK,
  IBI_MAX_HEAD,
  
  //IT_ARMOR
  IBI_TOP_ARMOR = IBI_MAX_HEAD,
  IBI_ROBE = IBI_TOP_ARMOR,
  IBI_LEATHERARMOR,
  IBI_CHAINMAIL,
  IBI_SPLINTMAIL,
  IBI_BREASTPLATE,
  IBI_PLATEMAIL,
  IBI_ETCHEDARMOR,
  IBI_LORDARMOR,
  IBI_MAX_ARMOR,

  //IT_FEET
  IBI_TOP_FEET = IBI_MAX_ARMOR,
  IBI_SHOES = IBI_TOP_FEET,
  IBI_LEATHERBOOTS,
  IBI_CHAINBOOTS,
  IBI_HEAVYBOOTS,
  IBI_GREAVES,
  IBI_MAX_FEET,

  //IT_HAND
  IBI_TOP_HAND = IBI_MAX_FEET,
  IBI_LEATHERGLOVE = IBI_TOP_HAND,
  IBI_CHAINGLOVE,
  IBI_GAUNTLETS,
  IBI_LORDGLOVES,
  IBI_MAX_HAND,

  //IT_RING
  IBI_TOP_RING = IBI_MAX_HAND,
  IBI_WOODRING = IBI_TOP_RING,
  IBI_IRONRING,
  IBI_ETCHEDRING,
  IBI_ENGRAVEDRING,
  IBI_MAX_RING,

  //IT_AMULET
  IBI_TOP_AMULET = IBI_MAX_RING,
  IBI_STONEAMULET = IBI_TOP_AMULET,
  IBI_AMULET,
  IBI_WOODAMULET,
  IBI_TALISMAN,
  IBI_MAX_AMULET,

  //IT_POTION
  IBI_TOP_POTION = IBI_MAX_AMULET,
  IBI_POTION_HEAL = IBI_TOP_POTION,
  IBI_MAX_POTION,

  //IT_SCROLL
  IBI_TOP_SCROLL = IBI_MAX_POTION,
  IBI_SCROLL_LIGHT = IBI_TOP_SCROLL,
  IBI_MAX_SCROLL,

  //IT_KEY
  IBI_TOP_KEY = IBI_MAX_SCROLL,
  IBI_IRONKEY = IBI_TOP_KEY,
  IBI_MAX_KEY,

  IBIMAX = IBI_MAX_KEY
};


typedef struct {
  ItemBaseIdx base; //基本情報
  uint8_t rank; //0:normal 1:rare 2:unique
  uint8_t prefix; //前置詞 index
  uint8_t suffix; //後置詞 index
  int16_t minatk; //最低攻撃力(base + x)
  int16_t maxatk; //最大攻撃力(base + x)
  int16_t def;    //守備力
  int16_t addhpmax; //体力最大値増加分
  int16_t addmpmax; //魔力最大値増加分
  int8_t  addatkspd; //攻撃速度増加
  int8_t  addmovspd; //移動速度増加
  ItemAttribute attr; //属性
  int16_t adddmg; //ダメージ増加率(%)
  int8_t addstr;
  int8_t adddex;
  int8_t addint;
  int8_t addluk;
  int8_t registfire; //耐火(%)
  int8_t registcold; //耐冷気(%)
  int8_t registphysic; //耐物理(%)
  int8_t registthunder; //耐電気(%)
  int8_t registmagic; //耐魔法(%)
  int8_t healhp; //回復量(%/sec)
  int8_t lightlvl; //明かりレベル
} ITEM;


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

extern ITEMBASE g_itembasetbl[];
inline ITEMBASE* itGetItemBase( ItemBaseIdx ibi )
{
  return &g_itembasetbl[ibi];
}
inline ITEMBASE* itGetItemBase( ITEM* item )
{
  return itGetItemBase( item->base );
}

/*
 * アイテムのランク
 *    normal: 通常
 *     magic: suffix or prefix
 *      rare: suffix & prefix
 *    unique: 固有
 */
enum : uint8_t {
  ITRANK_NORMAL,
  ITRANK_MAGIC,
  ITRANK_RARE,
  ITRANK_UNIQUE,
};

//item rank color
#define ITRCOLOR_NORMAL ColorIndex::white
#define ITRCOLOR_MAGIC  ColorIndex::lightblue
#define ITRCOLOR_RARE   ColorIndex::yellow
#define ITRCOLOR_UNIQUE ColorIndex::lightgreen


typedef struct {
  uint16_t droptype; //落ちる種類(IT_xxx)のbitのリスト
  uint8_t unique; //unique の出る割合(%)
  uint8_t rare;   //rare の出る割合(%)
  uint8_t magic;  //magic の出る割合(%)
} ITEMDROP;


/*
 * アイテムを生成する
 */
extern ITEM* itGenerate( ItemBaseIdx ibi, uint8_t itrank, uint8_t droplvl );
extern ITEM* itGenerateFloor( uint8_t mapfloor ); //床に落ちてるアイテム
extern ITEM* itGenerateContainer( uint8_t mapfloor, uint8_t droplvl ); //宝箱等の内のアイテム
extern ItemType itRandomItemType( uint16_t flags );
extern ItemBaseIdx itRandomItemBase( ItemType type );
extern ColorIndex itGetRankColor( uint8_t itrank );

extern void itMakeName( ITEM* item, char* buf );
extern const char* itGetPrefixName( ITEM* item );
extern const char* itGetSuffixName( ITEM* item );
extern const char* itGetBaseName( ITEM* item );
extern ColorIndex itGetRankColor( ITEM* item );

extern bool itSave( int16_t svslot, ITEM* item );
extern bool itSave( File& f, ITEM* item ); //x!x! まとめたい…
extern ITEM* itLoad( int16_t svslot );
extern ITEM* itLoad( File& f );


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

