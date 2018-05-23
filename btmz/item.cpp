#include "btmz.h"

#include "item.h"

#include "pic.h"



//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
enum : int8_t {
  AFT_DEF, //16
  AFT_MINATK, //16
  AFT_MAXATK, //16
  AFT_ATKSPD,
  AFT_MOVSPD,
  AFT_ADDHPMAX, //16
  AFT_ADDMPMAX, //16
  AFT_ADDDMG, //16
  AFT_ADDSTR,
  AFT_ADDDEX,
  AFT_ADDINT,
  AFT_ADDLUK,
  AFT_RGFIRE,
  AFT_RGCOLD,
  AFT_RGTHUNDER,
  AFT_RGMAGIC,
  AFT_HEALHP,
  AFT_LIGHTLVL,
  AFT_ATTR,
  AFTMAX
};



//3byte
#define AF8( type, base, rnd ) type, base, rnd
//5byte
#define AF16( type, base, rnd ) type, (base&0xff), ((base>>8)&0xff), (rnd&0xff), ((rnd>>8)&0xff)

#define AFDEF( base, rnd ) AF16( AFT_DEF, base, rnd )
#define AFMINATK( base, rnd ) AF16( AFT_MINATK, base, rnd )
#define AFMAXATK( base, rnd ) AF16( AFT_MAXATK, base, rnd )
#define AFATKSPD( base, rnd ) AF8( AFT_ATKSPD, base, rnd )
#define AFMOVSPD( base, rnd ) AF8( AFT_MOVSPD, base, rnd )
#define AFADDHPMAX( base, rnd ) AF16( AFT_ADDHPMAX, base, rnd )
#define AFADDMPMAX( base, rnd ) AF16( AFT_ADDMPMAX, base, rnd )
#define AFADDDMG( base, rnd ) AF16( AFT_ADDDMG, base, rnd )
#define AFADDSTR( base, rnd ) AF8( AFT_ADDSTR, base, rnd )
#define AFADDDEX( base, rnd ) AF8( AFT_ADDDEX, base, rnd )
#define AFADDINT( base, rnd ) AF8( AFT_ADDINT, base, rnd )
#define AFADDLUK( base, rnd ) AF8( AFT_ADDLUK, base, rnd )
#define AFRGFIRE( base, rnd ) AF8( AFT_RGFIRE, base, rnd )
#define AFRGCOLD( base, rnd ) AF8( AFT_RGCOLD, base, rnd )
#define AFRGTHUNDER( base, rnd ) AF8( AFT_RGTHUNDER, base, rnd )
#define AFRGMAGIC( base, rnd ) AF8( AFT_RGMAGIC, base, rnd )
#define AFHEALHP( base, rnd ) AF8( AFT_HEALHP, base, rnd )
#define AFLIGHTLVL( base, rnd ) AF8( AFT_LIGHTLVL, base, rnd )
#define AFATTR( attr ) AFT_ATTR, (base&0xff), ((base>>8)&0xff), ((base>>16)&0xff), ((base>>24)&0xff)
#define AFEND() AFTMAX


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

ITEMBASE g_itembasetbl[] = {
  //-- IT_WEAPON -----
  { //IBI_DAGGER
    IT_WEAPON, &ipDagger, &ipEqDagger,
    "Dagger",
    1, 4, 0,
    IUE_NOTHING, 0
  },
  { //IBI_SHORTSWORD
    IT_WEAPON, &ipShortSword, &ipEqShortSword,
    "Short Sword",
    1, 6, 0, //atkmin, atkmax, def
    IUE_NOTHING, 0
  },
  { //IBI_BLADE
    IT_WEAPON, &ipBlade, &ipEqBlade,
    "Blade",
    2, 5, 0,
    IUE_NOTHING, 0
  },
  { //IBI_BROADSWORD
     IT_WEAPON, &ipBroadSword, &ipEqBroadSword,
    "Broad Sword",
    2, 8, 0,
    IUE_NOTHING, 0
 },
  { //IBI_MACE
    IT_WEAPON, &ipMace, &ipEqMace,
    "Mace",
    5, 5, 0,
    IUE_NOTHING, 0
  },
  { //IBI_LONGSWORD
    IT_WEAPON, &ipLongSword, &ipEqLongSword,
    "Long Sword",
    4, 10, 0,
    IUE_NOTHING, 0
  },
  { //IBI_CUTLASS
    IT_WEAPON, &ipCutlass, &ipEqCutlass,
    "Cutlass",
    2, 12, 0,
    IUE_NOTHING, 0
  },
  { //IBI_WARMACE
    IT_WEAPON, &ipWarMace, &ipEqWarMace,
    "WarMace",
    15, 15, 0,
    IUE_NOTHING, 0
  },
  { //IBI_KATANA
    IT_WEAPON, &ipKatana, &ipEqKatana,
    "Katana",
    8, 20, 0,
    IUE_NOTHING, 0
  },
  { //IBI_RUNESWORD
    IT_WEAPON, &ipRuneSword, &ipEqRuneSword,
    "Rune Sword",
    8, 28, 0,
    IUE_NOTHING, 0
  },
  { //IBI_LORDSWORD
    IT_WEAPON, &ipLordSword, &ipEqLordSword,
    "Lord Sword",
    16, 32, 0,
    IUE_NOTHING, 0
  },
  //-- IT_SHEILD -----
  { //IBI_WOODSHIELD
    IT_SHIELD, &ipWoodShield, NULL,
    "Wood Shield",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_ROUNDSHIELD
    IT_SHIELD, &ipRoundShield, NULL,
    "RoundShield",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_BUCKLER
    IT_SHIELD, &ipRoundShield, NULL,
    "Buckler",
    0, 0, 2,
    IUE_NOTHING, 0
  },
  { //IBI_HEATERSHIELD
    IT_SHIELD, &ipRoundShield, NULL,
    "HeaterShield",
    0, 0, 3,
    IUE_NOTHING, 0
  },
  { //IBI_LARGESHIELD
    IT_SHIELD, &ipRoundShield, NULL,
    "LargeShield",
    0, 0, 4,
    IUE_NOTHING, 0
  },
  { //IBI_KITESHIELD
    IT_SHIELD, &ipRoundShield, NULL,
    "KiteShield",
    0, 0, 5,
    IUE_NOTHING, 0
  },
  { //IBI_TOWERSHIELD
    IT_SHIELD, &ipRoundShield, NULL,
    "TowerSheild",
    0, 0, 6,
    IUE_NOTHING, 0
  },
  //-- IT_HEAD -----
  { //IBI_LEATHERHELM
    IT_HEAD, &ipLeatherHelm, NULL,
    "LeatherHelm",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_HELM
    IT_HEAD, &ipHelm, NULL,
    "Helm",
    0, 0, 2,
    IUE_NOTHING, 0
  },
  { //IBI_CLOSEDHELM
    IT_HEAD, &ipHelm, NULL,
    "ClosedHelm",
    0, 0, 3,
    IUE_NOTHING, 0
  },
  { //IBI_CREATHELM
    IT_HEAD, &ipHelm, NULL,
    "GreatHelm",
    0, 0, 4,
    IUE_NOTHING, 0
  },
  { //IBI_BASINET
    IT_HEAD, &ipHelm, NULL,
    "Basinet",
    0, 0, 3,
    IUE_NOTHING, 0
  },
  { //IBI_COIF
    IT_HEAD, &ipHelm, NULL,
    "Coif",
    0, 0, 2,
    IUE_NOTHING, 0
  },
  { //IBI_SKULLCAP
    IT_HEAD, &ipHelm, NULL,
    "SkullCap",
    0, 0, 2,
    IUE_NOTHING, 0
  },
  { //IBI_CIRCLET
    IT_HEAD, &ipHelm, NULL,
    "Circlet",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_HOOD
    IT_HEAD, &ipHelm, NULL,
    "Hood",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_MASK
    IT_HEAD, &ipHelm, NULL,
    "Mask",
    0, 0, 2,
    IUE_NOTHING, 0
  },

  //-- IT_ARMOR -----
  { //IBI_ROBE
    IT_ARMOR, &ipLeatherArmor, NULL,
    "Robe",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_LEATHERARMOR
    IT_ARMOR, &ipLeatherArmor, NULL,
    "LeatherArmor",
    0, 0, 3,
    IUE_NOTHING, 0
  },
  { //IBI_CHAINMAIL
    IT_ARMOR, &ipChainMail, NULL,
    "ChainMail",
    0, 0, 4,
    IUE_NOTHING, 0
  },
  { //IBI_SPLINTMAIL
    IT_ARMOR, &ipChainMail, NULL,
    "SplintMail",
    0, 0, 5,
    IUE_NOTHING, 0
  },
  { //IBI_BREASTPLATE
    IT_ARMOR, &ipChainMail, NULL,
    "BreastPlate",
    0, 0, 5,
    IUE_NOTHING, 0
  },
  { //IBI_PLATEMAIL
    IT_ARMOR, &ipChainMail, NULL,
    "PlateMail",
    0, 0, 7,
    IUE_NOTHING, 0
  },
  { //IBI_ETCHEDARMOR
    IT_ARMOR, &ipChainMail, NULL,
    "EtchedArmor",
    0, 0, 8,
    IUE_NOTHING, 0
  },
  { //IBI_LORDARMOR
    IT_ARMOR, &ipChainMail, NULL,
    "LordArmor",
    0, 0, 10,
    IUE_NOTHING, 0
  },

  //-- IT_FEET -----
  { //IBI_SHOES
    IT_FEET, &ipShoes, NULL,
    "Shoes",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_LEATHERBOOTS
    IT_FEET, &ipLeatherBoots, NULL,
    "LeatherBoots",
    0, 0, 2,
    IUE_NOTHING, 0
  },
  { //IBI_CHAINBOOTS
    IT_FEET, &ipLeatherBoots, NULL,
    "ChainBoots",
    0, 0, 3,
    IUE_NOTHING, 0
  },
  { //IBI_HEAVYBOOTS
    IT_FEET, &ipLeatherBoots, NULL,
    "HeavyBoots",
    0, 0, 4,
    IUE_NOTHING, 0
  },
  { //IBI_GREAVES
    IT_FEET, &ipLeatherBoots, NULL,
    "Greaves",
    0, 0, 6,
    IUE_NOTHING, 0
  },
  
  //-- IT_HAND -----
  { //IBI_LEATHERGLOVE
    IT_HAND, &ipLeatherGlove, NULL,
    "LeatherGlove",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_CHAINGLOVE
    IT_HAND, &ipChainGlove, NULL,
    "ChainGlove",
    0, 0, 2,
    IUE_NOTHING, 0
  },
  { //IBI_GAUNTLETS
    IT_HAND, &ipChainGlove, NULL,
    "Gauntlets",
    0, 0, 4,
    IUE_NOTHING, 0
  },
  { //IBI_LORDGLOVES
    IT_HAND, &ipChainGlove, NULL,
    "LordGloves",
    0, 0, 6,
    IUE_NOTHING, 0
  },
  
  //-- IT_RING -----
  { //IBI_WOODRING
    IT_RING, &ipWoodRing, NULL,
    "Wood Ring",
    0, 0, 0,
    IUE_NOTHING, 0
  },
  { //IBI_IRONRING
    IT_RING, &ipIronRing, NULL,
    "Iron Ring",
    0, 0, 0,
    IUE_NOTHING, 0
  },
  { //IBI_ETCHEDRING
    IT_RING, &ipIronRing, NULL,
    "Etched Ring",
    0, 0, 0,
    IUE_NOTHING, 0
  },
  { //IBI_ENGRAVEDRING
    IT_RING, &ipIronRing, NULL,
    "EngravedRing",
    0, 0, 0,
    IUE_NOTHING, 0
  },

  //-- IT_AMULET -----
  { //IBI_STONEAMULET
    IT_AMULET, &ipStoneAmulet, NULL,
    "Stone Amulet",
    0, 0, 0,
    IUE_NOTHING, 0
  },
  { //IBI_AMULET
    IT_AMULET, &ipAmulet, NULL,
    "Amulet",
    0, 0, 0,
    IUE_NOTHING, 0
  },
  { //IBI_WOODAMULET
    IT_AMULET, &ipAmulet, NULL,
    "Wood Amulet",
    0, 0, 0,
    IUE_NOTHING, 0
  },
  { //IBI_TALISMAN
    IT_AMULET, &ipAmulet, NULL,
    "Talisman",
    0, 0, 0,
    IUE_NOTHING, 0
  },

  //-- IT_POTION -----
  { //IBI_POTION_HEAL
    IT_POTION, &ipLongSword, NULL,
    "Heal Potion",
    0, 0, 0,
    IUE_NOTHING, 0
  },

  //-- IT_SCROLL -----
  { //IBI_SCROLL
    IT_SCROLL, &ipLongSword, NULL,
    "Light Scroll",
    0, 0, 0,
    IUE_NOTHING, 0
  },

  //-- IT_KEY -----
  { //IBI_KEY
    IT_KEY, &ipLongSword, NULL,
    "Iron Key",
    0, 0, 0,
    IUE_NOTHING, 0
  },


};

//x!x! 装備の種類によってつく、つかないを設定する？
static const char* g_prefixname[ITPREFIXMAX] = { //7文字
  "",
  "Shiny ", "Light ", "Glowing", //light
  "Agile ", "Hard ", "Stable ",  //
  "Ogre ", "Titan ",
  "Holy ", "Devout ",
};
/*
static const int8_t g_pf_[] = {
  AFEND()
};
  マイナスのパラメータは、ベースをマイナスにしておくとランダムもマイナス方向のランダムになる
 */
static const int8_t g_pf_shiny[] = {
  AFLIGHTLVL( 1, 2 ),
  AFEND()
};
static const int8_t g_pf_light[] = {
  AFLIGHTLVL( 4, 3 ),
  AFEND()
};
static const int8_t g_pf_growing[] = {
  AFLIGHTLVL( 8, 6 ),
  AFEND()
};
static const int8_t g_pf_agile[] = {
  AFADDDEX( 2, 3 ),
  AFEND()
};
static const int8_t g_pf_hard[] = {
  AFDEF( 2, 4 ),
  AFEND()
};
static const int8_t g_pf_stable[] = {
  AFATKSPD( 1, 2 ),
  AFEND()
};
static const int8_t g_pf_ogre[] = {
  AFADDSTR( 2, 4 ),
  AFADDDMG( 5, 6 ),
  AFEND()
};
static const int8_t g_pf_titan[] = {
  AFADDSTR( 5, 6 ),
  AFADDDMG( 10, 11 ),
  AFEND()
};
static const int8_t g_pf_holy[] = {
  AFADDINT( 2, 3 ),
  AFHEALHP( 2, 3 ),
  AFEND()
};
static const int8_t g_pf_devout[] = {
  AFADDINT( 4, 4 ),
  AFHEALHP( 4, 4 ),
  AFEND()
};

static const int8_t* g_prefixdata[] = {
  //x!x! none の分は置かない
  g_pf_shiny, g_pf_light, g_pf_growing,
  g_pf_agile, g_pf_hard, g_pf_stable,
  g_pf_ogre, g_pf_titan,
  g_pf_holy, g_pf_devout,
};

/*
static const int8_t g_sf_[] = {
  AFEND()
};
  マイナスのパラメータは、ベースをマイナスにしておくとランダムもマイナス方向のランダムになる
 */
static const char* g_suffixname[ITSUFFIXMAX] = { //12文字
  "",  
  "of Moon", "of Star", "of Sun",
  "of Power", "of Force", 
  "of Mind", "of Soul",
  "of Healing", "of Recovery",
  "the King", "the Queen", "the Emperor",
  "of Fire", "of Flame", "of Cold", "of Ice",
};

static const int8_t g_sf_ofmoon[] = {
  AFADDSTR( 2, 4 ),
  AFADDDEX( 2, 4 ),
  AFADDINT( 2, 4 ),
  AFADDLUK( 2, 4 ),
  AFEND()
};
static const int8_t g_sf_ofstar[] = {
  AFADDSTR( 5, 4 ),
  AFADDDEX( 5, 4 ),
  AFADDINT( 5, 4 ),
  AFADDLUK( 5, 4 ),
  AFEND()
};
static const int8_t g_sf_ofsun[] = {
  AFADDSTR( 8, 3 ),
  AFADDDEX( 8, 3 ),
  AFADDINT( 8, 3 ),
  AFADDLUK( 8, 3 ),
  AFEND()
};

static const int8_t g_sf_ofpower[] = {
  AFADDDMG( 10, 10 ),
  AFEND()
};

static const int8_t g_sf_offorce[] = {
  AFADDDMG( 20, 10 ),
  AFEND()
};
static const int8_t g_sf_ofmind[] = {
  AFADDMPMAX( 5, 6 ),
  AFEND()
};
static const int8_t g_sf_ofsoul[] = {
  AFADDMPMAX( 10, 11 ),
  AFEND()
};
static const int8_t g_sf_ofhealing[] = {
  AFHEALHP( 3, 4 ),
  AFEND()
};
static const int8_t g_sf_ofrecovery[] = {
  AFHEALHP( 5, 4 ),
  AFEND()
};
static const int8_t g_sf_theking[] = {
  AFMINATK( 2, 4 ),
  AFADDDMG( 2, 4 ),
  AFATKSPD( 2, 4 ),
  AFEND()
};
static const int8_t g_sf_thequeen[] = {
  AFMINATK( 5, 4 ),
  AFADDDMG( 5, 4 ),
  AFATKSPD( 5, 4 ),
  AFEND()
};
static const int8_t g_sf_theemperor[] = {
  AFMINATK( 8, 5 ),
  AFADDDMG( 8, 5 ),
  AFATKSPD( 8, 5 ),
  AFEND()
};
static const int8_t g_sf_offire[] = {
  AFRGFIRE( 3, 3 ),
  AFEND()
};
static const int8_t g_sf_offlame[] = {
  AFRGFIRE( 5, 4 ),
  AFEND()
};
static const int8_t g_sf_ofcold[] = {
  AFRGCOLD( 3, 3 ),
  AFEND()
};
static const int8_t g_sf_ofice[] = {
  AFRGCOLD( 5, 4 ),
  AFEND()
};

static const int8_t* g_suffixdata[] = {
  //x!x! none の分は置かない
  g_sf_ofmoon,
  g_sf_ofstar,
  g_sf_ofsun,
  g_sf_ofpower,
  g_sf_offorce,
  g_sf_ofmind,
  g_sf_ofsoul,
  g_sf_ofhealing,
  g_sf_ofrecovery,
  g_sf_theking,
  g_sf_thequeen,
  g_sf_theemperor,
  g_sf_offire,
  g_sf_offlame,
  g_sf_ofcold,
  g_sf_ofice,
};

static void setupAffix( ITEM* item );
static void applyAffix( ITEM* item, const int8_t* data );

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
/*
 * マップに落ちているアイテムの生成
 * 
 */
static ITEMDROP g_floordrop[] = {
  //unique(%), rare(%), magic(%)
  { ITB_EQMAIN(), 0, 1, 10 },  //1 - 2
  { ITB_EQMAIN(), 0, 3, 20 }, //3 - 4
  { ITB_EQALL(), 0, 5, 25 }, //5 - 6
  { ITB_EQALL(), 2, 10, 35 }, //7 - 8
  { ITB_EQALL(), 2, 15, 45 }, //9 - 10
  { ITB_EQALL(), 4, 20, 50 }, //11 - 12
  { ITB_EQALL(), 4, 25, 50 }, //13 - 14
  { ITB_EQALL(), 6, 30, 50 }, //15 - 16
  { ITB_EQALL(), 6, 35, 50 }, //17 - 18
  { ITB_EQALL(), 8, 40, 50 }, //19 - 20
};
ITEM* itGenerateFloor( uint8_t mapfloor )
{
  uint8_t itrank = ITRANK_NORMAL;

  uint8_t flvl = (mapfloor-1) / 2;
  if( flvl >= (sizeof(g_floordrop)/sizeof(ITEMDROP)) )
    flvl = (sizeof(g_floordrop)/sizeof(ITEMDROP)) - 1;

  uint8_t r = random(100);
  uint8_t cl = g_floordrop[flvl].unique;
  
  if( r < cl ) itrank = ITRANK_UNIQUE;
  else {
    cl += g_floordrop[flvl].rare;
    if( r < cl ) itrank = ITRANK_RARE;
    else {
      cl += g_floordrop[flvl].magic;
      if( r < cl ) itrank = ITRANK_MAGIC;
    }
  }

  //アイテムのタイプをフラグに基づきランダム選択
  ItemType it = itRandomItemType( g_floordrop[flvl].droptype );

  //指定のタイプの中からランダムに選択
  ItemBaseIdx ibi = itRandomItemBase( it );

  uint8_t droplvl = 1;

  return itGenerate( ibi, itrank, droplvl );
}

ITEM* itGenerateContainer( uint8_t mapfloor, uint8_t droplvl )
{
  //x!x! 仮 
  return itGenerateFloor( mapfloor );
}

ItemType itRandomItemType( uint16_t flags )
{
  uint8_t sel[ITMAX];
  int8_t selcnt = 0;

  //選んで良いのを集める
  for( int8_t i=0; i<ITMAX; i++ ) {
    if( flags & (1<<i) ) {
      sel[selcnt++] = i;
    }
  }

  return ItemType( sel[ random(selcnt) ] );
}

ItemBaseIdx itRandomItemBase( ItemType type )
{
  int16_t ibtop, ibend, ibsize;
  switch( type ) {
    case IT_WEAPON: ibtop = IBI_TOP_WEAPON; ibend = IBI_MAX_WEAPON; break;
    case IT_SHIELD: ibtop = IBI_TOP_SHIELD; ibend = IBI_MAX_SHIELD; break;
    case IT_HEAD: ibtop = IBI_TOP_HEAD; ibend = IBI_MAX_HEAD; break;
    case IT_ARMOR: ibtop = IBI_TOP_ARMOR; ibend = IBI_MAX_ARMOR; break;
    case IT_FEET: ibtop = IBI_TOP_FEET; ibend = IBI_MAX_FEET; break;
    case IT_HAND: ibtop = IBI_TOP_HAND; ibend = IBI_MAX_HAND; break;
    case IT_RING: ibtop = IBI_TOP_RING; ibend = IBI_MAX_RING; break;
    case IT_AMULET: ibtop = IBI_TOP_AMULET; ibend = IBI_MAX_AMULET; break;
    case IT_POTION: ibtop = IBI_TOP_POTION; ibend = IBI_MAX_POTION; break;
    case IT_SCROLL: ibtop = IBI_TOP_SCROLL; ibend = IBI_MAX_SCROLL; break;
    case IT_KEY: ibtop = IBI_TOP_KEY; ibend = IBI_MAX_KEY; break;
  }

  ibsize = ibend - ibtop;
  int16_t s = random(ibsize);

  return ItemBaseIdx(ibtop + s);
}


/*
 * droplvl = floor / 10 + monster / 10 とか？
 */
ITEM* itGenerate( ItemBaseIdx ibi, uint8_t itrank, uint8_t droplvl )
{
  //normal or else

  
  ITEMBASE *ib = &g_itembasetbl[ibi];

  ITEM* item = new ITEM;
  memset( item, 0, sizeof(ITEM) );
  
  item->base = ibi;
  item->rank = itrank;
  item->prefix = ITPREFIX_NONE;
  item->suffix = ITSUFFIX_NONE;
  item->minatk = ib->minatk;
  item->maxatk = ib->maxatk;
  item->def = ib->def;
  
  //rank によって特殊効果
  bool prefix = false;
  bool suffix = false;
  switch( itrank ) {
    case ITRANK_NORMAL:
      break;
    case ITRANK_MAGIC:
      (random(100) & 1) ? (prefix = true) : (suffix = true);
      break;
    case ITRANK_RARE:
      prefix = suffix = true;
      break;
    case ITRANK_UNIQUE:
      break;
  }

    
  //x!x! affix にレベルつけて、出るものを絞る？
  //Prefix
  if( prefix ) {
    item->prefix = 1+random(ITPREFIXMAX-1);
  }
  //Suffix
  if( suffix ) {
    item->suffix = 1+random(ITSUFFIXMAX-1);
  }
  setupAffix(item); //affixの効果を反映


  return item;
}

void setupAffix( ITEM* item )
{

  if( item->prefix != ITPREFIX_NONE ) {
    applyAffix( item, g_prefixdata[item->prefix-1] );
  }
  if( item->suffix != ITSUFFIX_NONE ) {
    applyAffix( item, g_suffixdata[item->suffix-1] );    
  }
}

void applyAffix( ITEM* it, const int8_t* ad )
{
  /*
  AFT_DEF, //16
  AFT_MINATK, //16
  AFT_MAXATK, //16
  AFT_ATKSPD,
  AFT_MOVSPD,
  AFT_ADDHPMAX, //16
  AFT_ADDDMG, //16
  AFT_ADDSTR,
  AFT_ADDDEX,
  AFT_ADDINT,
  AFT_ADDLUK,
  AFT_RGFIRE,
  AFT_RGCOLD,
  AFT_RGTHUNDER,
  AFT_RGMAGIC,
  AFT_HEALHP,
  AFT_LIGHTLVL,
  AFT_ATTR,
   */
  while( *ad != AFTMAX ) {
    int16_t* v16=NULL;
    int8_t* v8=NULL;
    uint32_t* v32=NULL;
    switch( *ad++ ) {
      case AFT_DEF: v16 = &it->def; break;
      case AFT_MINATK: v16 = &it->minatk; break;
      case AFT_MAXATK: v16 = &it->maxatk; break;
      case AFT_ATKSPD: v8 = &it->addatkspd; break;
      case AFT_MOVSPD: v8 = &it->addmovspd; break;
      case AFT_ADDHPMAX: v16 = &it->addhpmax; break;
      case AFT_ADDDMG: v16 = &it->adddmg; break;
      case AFT_ADDSTR: v8 = &it->addstr; break;
      case AFT_ADDDEX: v8 = &it->adddex; break;
      case AFT_ADDINT: v8 = &it->addint; break;
      case AFT_ADDLUK: v8 = &it->addluk; break;
      case AFT_RGFIRE: v8 = &it->registfire; break;
      case AFT_RGCOLD: v8 = &it->registcold; break;
      case AFT_RGTHUNDER: v8 = &it->registthunder; break;
      case AFT_RGMAGIC: v8 = &it->registmagic; break;
      case AFT_HEALHP: v8 = &it->healhp; break;      
      case AFT_LIGHTLVL: v8 = &it->lightlvl; break;
      case AFT_ATTR: v32 = reinterpret_cast<uint32_t*>(&it->attr); break;
    }

    if( v8 ) {
      *v8 = (*ad++);
      if( *v8 >= 0 ) *v8 += random( *ad++ );
      else           *v8 -= random( *ad++ );
    } else
    if( v16 ) {
        int16_t b, r;
        b = ((*ad++)&0xff) | ((*ad++) << 8);
        r = ((*ad++)&0xff) | ((*ad++) << 8);
        *v16 = b;
        if( *v16 >= 0 ) *v16 += random( r );
        else            *v16 -= random( r );
    } else
    if( v32 ) {
    }
  }
}


void itMakeName( ITEM* item, char* buf )
{
  sprintf( buf, "%s%s%s", itGetPrefixName(item), itGetBaseName(item), itGetSuffixName(item) );
}

const char* itGetPrefixName( ITEM* item )
{
  return g_prefixname[ item->prefix ];
}

const char* itGetSuffixName( ITEM* item )
{
  return g_suffixname[ item->suffix ];
}

const char* itGetBaseName( ITEM* item )
{
  ITEMBASE *ib = &g_itembasetbl[item->base];

  return ib->name;
}

ColorIndex itGetRankColor( uint8_t itrank ) 
{
  ColorIndex ci;
  switch( itrank ) {
    case ITRANK_NORMAL: ci = ITRCOLOR_NORMAL; break;
    case ITRANK_MAGIC: ci = ITRCOLOR_MAGIC; break;
    case ITRANK_RARE: ci = ITRCOLOR_RARE; break;
    case ITRANK_UNIQUE: ci = ITRCOLOR_UNIQUE; break;
    default: ci = ColorIndex::white; break;
  }
  return ci;
}

ColorIndex itGetRankColor( ITEM* item ) 
{
  return itGetRankColor( item->rank );
}

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
bool itSave( int16_t svslot, ITEM* item )
{
  if( item ) {
    gb.save.set( svslot, item, sizeof(ITEM) );
  } else {
    ITEM t;
    t.base = IBI_UNDEFINED;
    gb.save.set( svslot, &t, sizeof(ITEM) );
  }

  return true;
}

bool itSave( File& f, ITEM* item )
{
  if( item ) {
    f.write( item, sizeof(*item) );
  } else {
    ITEM t;
    t.base = IBI_UNDEFINED;
    f.write( &t, sizeof(t));
  }
}

ITEM* itLoad( int16_t svslot )
{
  ITEM* it = new ITEM;

  gb.save.get( svslot, it, sizeof(ITEM) );
  if( it->base == IBI_UNDEFINED ) {
    //アイテムの無い場所 or 無効なアイテム
    delete it;
    it = NULL;
  }

  return it;
}

ITEM* itLoad( File& f )
{
  ITEM* it = new ITEM;

  f.read( it, sizeof(*it) );
  if( it->base == IBI_UNDEFINED ) {
    //アイテムの無い場所 or 無効なアイテム
    delete it;
    it = NULL;
  }

  return it;
}

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

