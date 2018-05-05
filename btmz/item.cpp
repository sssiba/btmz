#include "btmz.h"

#include "item.h"

#include "pic.h"

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

ITEMBASE g_itembasetbl[] = {
  //-- IT_WEAPON -----
  { //IBI_SHORTSWORD
    IT_WEAPON, &ipShortSword, &ipEqShortSword,
    "ShortSword",
    1, 6, 0, //atkmin, atkmax, def
    IUE_NOTHING, 0
  },
  { //IBI_LONGSWORD
    IT_WEAPON, &ipLongSword, &ipEqLongSword,
    "LongSword",
    1, 8, 0,
    IUE_NOTHING, 0
  },
  { //IBI_DAGGER
    IT_WEAPON, &ipLongSword, &ipEqLongSword,
    "Dagger",
    1, 4, 0,
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
    0, 0, 1,
    IUE_NOTHING, 0
  },

  //-- IT_ARMOR -----
  { //IBI_LEATHERARMOR
    IT_ARMOR, &ipLeatherArmor, NULL,
    "LeatherArmor",
    0, 0, 3,
    IUE_NOTHING, 0
  },
  { //IBI_CHAINMAIL
    IT_ARMOR, &ipChainMail, NULL,
    "Chain Mail",
    0, 0, 4,
    IUE_NOTHING, 0
  },

  //-- IT_FOOT -----
  { //IBI_LEATHERBOOTS
    IT_FOOT, &ipLeatherBoots, NULL,
    "LeatherBoots",
    0, 0, 1,
    IUE_NOTHING, 0
  },
  { //IBI_SHOES
    IT_FOOT, &ipShoes, NULL,
    "Shoes",
    0, 0, 1,
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
    "Chain Glove",
    0, 0, 1,
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

static const char* g_prefixname[ITPREFIXMAX] = {
  "",
  "Red ", "Blue ", "Shiny ",
};

static const char* g_suffixname[ITSUFFIXMAX] = {
  "",  
  "of Soul", "of Moon", "of Star",
};

static void setupAffix( ITEM* item );

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
/*
 * マップに落ちているアイテムの生成
 * 
 */
static ITEMDROP g_floordrop[] = {
  //unique(%), rare(%), magic(%)
  { ITB_EQMAIN(), 0, 1, 5 },  //1 - 2
  { ITB_EQMAIN(), 0, 3, 15 }, //3 - 4
  { ITB_EQALL(), 0, 5, 20 }, //5 - 6
  { ITB_EQALL(), 2, 10, 30 }, //7 - 8
  { ITB_EQALL(), 2, 15, 40 }, //9 - 10
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
    case IT_FOOT: ibtop = IBI_TOP_FOOT; ibend = IBI_MAX_FOOT; break;
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
  
  item->base = ibi;
  item->prefix = ITPREFIX_NONE;
  item->suffix = ITSUFFIX_NONE;
  item->minatk = ib->minatk;
  item->maxatk = ib->maxatk;
  item->def = ib->def;
  item->addhpmax = 0;
  item->addatkspd = 0;
  item->addmovspd = 0;
  item->rank = itrank;
  item->atr = ItemAttribute( 0 );

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

