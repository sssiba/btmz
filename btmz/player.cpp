#include "player.h"

#include "pic.h"

#include "dungeon.h"

#include "enemy.h"

#include "item.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
int16_t g_plx, g_ply;
static bool g_plflip;
static uint8_t g_planm;
static uint8_t g_planmwait;
static uint8_t g_plphase;
static uint8_t g_plmode;
static uint8_t g_plwait;
static uint8_t g_plwkflag; //ワーク的に使用するフラグ
//static int16_t g_plhpmax;
//static int16_t g_plhp;
//static int16_t g_plstr;
//static int16_t g_pldef;
Rect8 g_plmvrect;
Rect8 g_platrect;
Rect8 g_pldfrect;
PLSTAT g_plstat;

enum : uint8_t {
  PLMODE_UNDEFINED,
  PLMODE_MOVE, //移動モード
  PLMODE_ATTACK, //攻撃モード
  PLMODE_ACTION, //行動モード
};

//ワーク的なフラグ。処理中に一時的に必要なフラグ。
enum : uint8_t {
  WF_DONEATTACKCHK = (1<<0),  //攻撃判定を完了した。
};

enum : uint8_t {
  PLPHASE_MV_WAIT,
  PLPHASE_MV_RIGHT,
  PLPHASE_MV_LEFT,
  PLPHASE_AT_WAIT, //攻撃モード待機
  PLPHASE_AT_ATTACK, //攻撃モード攻撃中（当たり判定発生中)
  PLPHASE_AT_COOLDOWN, //攻撃モード攻撃後硬直
  PLPHASE_AC_WAIT, //行動モード待機
};

#if defined( DBG_MAP )
uint8_t DBGtoarea, DBGtoblock;
#endif

//中心位置。左上からのオフセット
#define PLCX 4
#define PLCY 16
#define PLPICW 8
#define PLPICH 16



static bool enter( uint8_t bdir, int16_t cx, int16_t cy );
static void modeMove();
static void modeMoveInit();
static void modeMoveFinish();
static void modeMoveDraw();
static void modeAttack();
static void modeAttackInit();
static void modeAttackFinish();
static void modeAttackDraw();
static void modeAction();
static void modeActionInit();
static void modeActionFinish();
static void modeActionDraw();

static void enterMode( uint8_t newmode );

static void plDrawStat();

static void applyItem(ITEM* item);
static void removeItem(ITEM* item);

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void plInit()
{
  g_plflip = false;
  g_plmode = PLMODE_UNDEFINED;
  enterMode( PLMODE_MOVE );

  //移動用当たり判定
  g_plmvrect.x = -4;
  g_plmvrect.y = -16;
  g_plmvrect.w = 8;
  g_plmvrect.h  = 16;

  //ダメージ用当たり判定
  g_pldfrect = g_plmvrect;

  //攻撃用当たり判定
  g_platrect.x = 4;
  g_platrect.y = -16;
  g_platrect.w = 4;
  g_platrect.h = 16;

  g_plwkflag = 0;

  g_plstat.hpmax = g_plstat.hp = 20;
  g_plstat.lvl = 1;
  g_plstat.str = 8;
  g_plstat.dex = 4;
  g_plstat.intl = 4;
  g_plstat.luk = 4;
  g_plstat.gold = 0;
  g_plstat.mp = 0;
  g_plstat.mpmax = 0;
  g_plstat.ex = 0;
  for( int8_t i=0; i<EQMAX; i++ ) {
    g_plstat.equip[i] = NULL;
  }
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    g_plstat.items[i] = NULL;
  }

  g_plstat.minatk = 0;
  g_plstat.maxatk = 0;
  g_plstat.def = 0;
  g_plstat.healhp = 0;

  //x!x! 適当な武器を装備させておく
  ITEM* weapon = itGenerate( IBI_SHORTSWORD, ITRANK_MAGIC, 10 );
  plEquip( weapon );
  ITEM* armor = itGenerate( IBI_LEATHERARMOR, ITRANK_NORMAL, 10 );
  plEquip( armor );

  plAddItem( itGenerateFloor(0) );
  plAddItem( itGenerateFloor(2) );
  plAddItem( itGenerateFloor(4) );
  plAddItem( itGenerateFloor(6) );
  plAddItem( itGenerateFloor(8) );
  plAddItem( itGenerateFloor(10) );
  
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
  plAddItem( itGenerateFloor(10) );
}

void plUpdate()
{
  if( gamemain.isPress( BUTTON_A ) ) {
    enterMode( PLMODE_ATTACK );
  } else
  if( gamemain.isPress( BUTTON_B ) ) {
    enterMode( PLMODE_ACTION );
  } else {
    enterMode( PLMODE_MOVE );
  }
  
  switch( g_plmode ) {
    case PLMODE_MOVE: modeMove(); break;
    case PLMODE_ATTACK: modeAttack(); break;
    case PLMODE_ACTION: modeAction(); break;
  }
}

void enterMode( uint8_t newmode )
{
  if( g_plmode == newmode ) return; //既に同じモード

  //以前のモードの後始末
  switch( g_plmode ) {
    case PLMODE_MOVE: modeMoveFinish(); break;
    case PLMODE_ATTACK: modeAttackFinish(); break;
    case PLMODE_ACTION: modeActionFinish(); break;
  }

  //新しいモードの初期化
  switch( newmode ) {
    case PLMODE_MOVE: modeMoveInit(); break;
    case PLMODE_ATTACK: modeAttackInit(); break;
    case PLMODE_ACTION: modeActionInit(); break;
  }
  
}

//------------------------------------------
void modeMoveInit()
{
  g_planm = 0;
  g_planmwait = 0;
  g_plmode = PLMODE_MOVE;
  g_plphase = PLPHASE_MV_WAIT;
}

void modeMoveFinish()
{
}

void modeMove()
{
  int16_t hx, hy;
  uint8_t plact = g_plphase; //プレイヤーの挙動。待機中、右移動中、左移動中、...
  int16_t cx, cy;
  cx = g_plx;
  cy = g_ply;
  cx += g_plflip ? TOFIX(-PLCX) : TOFIX(PLCX); //判定を行う位置は、画像の進行方向側の端。

  DUNMAP()->getHome( hx, hy );

  if( gamemain.isTrigger( BUTTON_UP ) ) {
    if( enter( BDIR_FAR, cx, cy ) ) return; //奥の入口に入る
  } else
  if( gamemain.isTrigger( BUTTON_DOWN ) ) {
    if( enter( BDIR_NEAR, cx, cy ) ) return; //手前の入口に入る
  }
  
  int16_t mx = 0;
  int16_t spd = TOFIX(0.75f);
  int16_t mv = 0;
  
  if( gamemain.isPress( BUTTON_LEFT ) ) {
    mv = -spd;
    g_plflip = true;
    plact = PLPHASE_MV_LEFT;
  } else
  if( gamemain.isPress( BUTTON_RIGHT ) ) {
    mv = spd;
    g_plflip = false;
    plact = PLPHASE_MV_RIGHT;
  } else {
    plact = PLPHASE_MV_WAIT;
  }

  //移動可能チェック
  //とりあえず BG を見る
  cx += mv;
  bool movable = true;
  uint8_t bg = 0;

  //エリアの端では LEFT, RIGHT の通路へ出入り
  if( (plact == PLPHASE_MV_LEFT) && enter( BDIR_LEFT, cx, cy ) ) return;
  if( (plact == PLPHASE_MV_RIGHT) && enter( BDIR_RIGHT, cx, cy ) ) return;
  
  //そうで無ければ移動可能か調べる
  bg = DUNMAP()->getMapBG( TOINT(cx), TOINT(cy) );
  movable = (bg == 0 );


  //敵にぶつかるか調べる
  {
    int16_t tx, ty;
    tx = g_plx + mv;
    ty = g_ply;
    EnemyData* ed = enCheckMvRect( TOINT(tx), TOINT(ty), g_plmvrect, g_plflip );
    if( ed ) {
      movable = false;
    }
  }


  
  

#if 1
  {
#if defined( DBG_MAP )
    gb.display.setColor( ColorIndex::gray );
    gb.display.setCursor( 0, 6 );
    char s[64];
    sprintf( s, PSTR("%d[%d]"), bg, movable );
    gb.display.print( s );

    {
      Area* ca = DUNMAP()->getCurArea();
      sprintf( s, PSTR("%d,%d->%d"), ca->DBGgetSX(), ca->DBGgetSY(), ca->DBGgetDir() );
      gb.display.setCursor( 40, 6 );
      gb.display.print( s );
    }
#endif
    
  }
#endif

  if( movable ) {
    g_plx += mv;
  }

#define SCROLLMARGIN 16

  //背景スクロール
  int16_t scrx = DUNMAP()->toScrX( plGetX() );
  if( scrx >= (80-SCROLLMARGIN)-4 ) {
    hx += scrx - ((80-SCROLLMARGIN)-4 - 1);
    DUNMAP()->setHomeX( hx );
  } else
  if( scrx < (SCROLLMARGIN)+4 ) {
    hx -= (SCROLLMARGIN+4) - scrx;
    DUNMAP()->setHomeX( hx );
  }

  //animation
  //左右は反転使う？
  if( plact == g_plphase ) { //行動が変わってない？
    if( ++g_planmwait == 3 ) {
      switch( g_plphase ) {
        case PLPHASE_MV_WAIT: g_planm = 4; break; //とりあえずアニメーションしない
        case PLPHASE_MV_RIGHT: g_planm = (g_planm + 1) & 0x3; break;
        case PLPHASE_MV_LEFT: g_planm = (g_planm +1) &0x3; break;
      }
      g_planmwait = 0;
    }
  } else {
    //行動が変わった
    g_plphase = plact;
    switch( g_plphase ) {
      case PLPHASE_MV_WAIT: g_planm = 4; break;
      case PLPHASE_MV_RIGHT: g_planm = 0; break;
      case PLPHASE_MV_LEFT: g_planm = 0; break; //逆は反転で
    }
    g_planmwait = 0;
  }
  
}

//------------------------------------------
void modeAttackInit()
{
  g_planm = 0;
  g_planmwait = 0;
  g_plmode = PLMODE_ATTACK;
  g_plphase = PLPHASE_AT_WAIT;
  g_plwait = 0;
}

void modeAttackFinish()
{
  
}

void modeAttack()
{
  uint8_t phase = g_plphase;

  switch( phase ) {
    case PLPHASE_AT_WAIT:
      {
        if( (!g_plflip && gamemain.isPress( BUTTON_RIGHT )) ||
            (g_plflip && gamemain.isPress( BUTTON_LEFT))
          ) { //x!x! 向いてる方向で変える？
          g_plwkflag &= ~WF_DONEATTACKCHK;
          g_plwait = 4;
          phase = PLPHASE_AT_ATTACK;
        }
      }
      break;
    case PLPHASE_AT_ATTACK:
      {
        //攻撃判定
        {
          if( !(g_plwkflag & WF_DONEATTACKCHK) ) {
            int16_t tx, ty;
            tx = g_plx;
            ty = g_ply;
            EnemyData* ed = enCheckDfRect( TOINT(tx), TOINT(ty), g_platrect, g_plflip );
            if( ed ) {
              int16_t dmg = enCalcDamage( ed );
              bool dead = enDamage( ed, dmg );
            }
            g_plwkflag |= WF_DONEATTACKCHK; //判定は攻撃１回につき１回だけ
          }
        }
        
        //時間が来るまで phase は変わらないが、当たり判定は1回行なったら残り時間では行わない
        if( --g_plwait == 0 ) {
          g_plwait = 5;
          phase = PLPHASE_AT_COOLDOWN;
        }
      }
      break;
    case PLPHASE_AT_COOLDOWN:
        if( --g_plwait == 0 ) {
          phase = PLPHASE_AT_WAIT;
        }
      break;  
  }

  if( phase != g_plphase ) {

    g_plphase = phase;
  }
  
}

//------------------------------------------
void modeActionInit()
{
  g_planm = 0;
  g_planmwait = 0;
  g_plmode = PLMODE_ACTION;
  g_plphase = PLPHASE_AC_WAIT;
}

void modeActionFinish()
{
  
}

void modeAction()
{
  
}


//------------------------------------------
void plDraw()
{
  switch( g_plmode ) {
    case PLMODE_MOVE: modeMoveDraw(); break;
    case PLMODE_ATTACK: modeAttackDraw(); break;
    case PLMODE_ACTION: modeActionDraw(); break;
  }

  plDrawStat();


#if defined( DBG_MAP )
    {
      char s[64];
      sprintf( s, PSTR("%d: (%d,%d) %d_%d"), DUNMAP()->getCurAreaIdx(), plGetX(), plGetY(), DBGtoarea, DBGtoblock );
      gb.display.setCursor( 0, 12 );
      gb.display.print( s );
    }
#endif

}


void modeMoveDraw()
{
  static const uint8_t frmtbl[] = { 1, 2, 3, 2, 0 };
  static const int8_t wppnt[] = { //各フレームにおける武器配置位置 offset (x の中央(-PLCX前の値)からのオフセット)
    //x(横幅の中央からのオフセット),y(最上部からのオフセット)
    -1, 8, //frm 0
    4, 6, //frm 1
    2, 7, //frm 2
    -2, 7, //frm 3
  };

  int16_t x, y;
  int16_t lx, ly;
  x = DUNMAP()->toScrX(TOINT(g_plx));
  y = DUNMAP()->toScrY(TOINT(g_ply));

  uint8_t frm = frmtbl[g_planm];
  
  getPic( PIC_CHARACTER )->setFrame( frm );
  gb.display.drawImage( x-PLCX, y-PLCY, *getPic( PIC_CHARACTER ), g_plflip ? -PLPICW : PLPICW, PLPICH );
    
//  FBL().setLight( DUNMAP()->toScrX(plGetX()), DUNMAP()->toScrY(plGetY())-8, 28 );

  //武器があったら描画
  ITEM* weapon = plGetEqWeapon();
  if( weapon ) {
    ITEMBASE* ib = itGetItemBase( weapon->base );
    ItemPic* ip = ib->eqpic;
    if( g_plflip ) {
      x -= wppnt[ frm*2 + 0 ];
      lx = x;
      x -= (ip->w + ip->x);
    } else {
      x += wppnt[ frm*2 + 0 ];
      lx = x;
      x += ip->x;
    }
    y += wppnt[ frm*2 + 1 ] + ip->y;
    ly = y - ip->y;
    ip->pic->setFrame( ip->frm );
    gb.display.drawImage( x, y-PLCY, *(ip->pic), g_plflip ? -ip->w:ip->w, ip->h );

//    FBL().setLight( lx, ly-PLCY, 20, fbIllumination::LVL_8 );
  }

}

void modeAttackDraw()
{
  int16_t frm = 4;

  switch( g_plphase ) {
    case PLPHASE_AT_WAIT: frm = 4; break;
    case PLPHASE_AT_ATTACK:
    case PLPHASE_AT_COOLDOWN:
      frm = 5;
      break;
  }

  int16_t x, y;
  int16_t lx, ly;
  x = DUNMAP()->toScrX(TOINT(g_plx));
  y = DUNMAP()->toScrY(TOINT(g_ply));
  
  getPic( PIC_CHARACTER )->setFrame( frm );
  gb.display.drawImage( x-PLCX, y-PLCY, *getPic( PIC_CHARACTER ), g_plflip ? -PLPICW : PLPICW, PLPICH );

  //武器があったら描画
  ITEM* weapon = plGetEqWeapon();
  if( weapon ) {
    ITEMBASE* ib = itGetItemBase( weapon->base );
    
    static const int8_t wppnt[] = { //各フレームにおける武器配置位置 offset (x の中央(-PLCX前の値)からのオフセット)
      //x(横幅の中央からのオフセット),y(最上部からのオフセット)
      0, 7, //frm 4
      4, 6, //frm 5
    };
    frm -= 4;
    ItemPic* ip = ib->eqpic;
    if( g_plflip ) {
      x -= wppnt[ frm*2 + 0 ];
      lx = x;
      x -= (ip->w + ip->x);
    } else {
      x += wppnt[ frm*2 + 0 ];
      lx = x;
      x += ip->x;
    }
    y += wppnt[ frm*2 + 1 ] + ip->y;
    ly = y - ip->y;
    ip->pic->setFrame( ip->frm );
    gb.display.drawImage( x, y-PLCY, *(ip->pic), g_plflip ? -ip->w:ip->w, ip->h );

  
//    FBL().setLight( lx, ly-PLCY, 20, fbIllumination::LVL_8 );
  }

}

void modeActionDraw()
{

}

void plDrawStat()
{
    gb.display.setColor( Color::white );
    gb.display.setCursor( 0, 0 );
    char s[32];
    sprintf( s, PSTR("H:%d/%d"), g_plstat.hp, g_plstat.hpmax );
    gb.display.print( s );
  
}

void plFinish()
{
  for( int8_t i=0; i<EQMAX; i++ ) {
    if( g_plstat.equip[i] ) {
      delete g_plstat.equip[i];
      g_plstat.equip[i] = NULL;
    }
  }
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    if( g_plstat.items[i] ) {
      delete g_plstat.items[i];
      g_plstat.items[i] = NULL;
    }
  }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
bool enter( uint8_t bdir, int16_t cx, int16_t cy )
{
  //入れるか
  //とりあえず BG を見る
  bool ok = false;
  cx = TOINT(cx);
  cy = TOINT(cy);

  switch( bdir ) {
    case BDIR_FAR:  //上下の出入り
    case BDIR_NEAR:
      {
        cy += getDirY( bdir ) * TILEH;
        uint8_t bg = DUNMAP()->getMapBG( cx, cy );
        if( bg == 4 || bg == 7 || bg == 0 
        || bg == 19 || bg == 21 //奥のドア
        || bg == 23 || bg == 24 //手前のドア
        ) {
          ok = true;
        }
      }
      break;
    case BDIR_LEFT: //左右の出入り
    case BDIR_RIGHT:
      {
        if( ((TOINT(g_plx)-4) < 0) || ((TOINT(g_plx)+4) >= (DUNMAP()->getCurArea()->getWidth())) ) {
          char s[64];
          sprintf( s, "enter pos [%d] %d  %d", bdir, g_plx, DUNMAP()->getCurArea()->getWidth() );
          TRACE( s );
          
           ok = true;
        } else {
          uint8_t bg = DUNMAP()->getMapBG( cx, cy );
          if( bg == 28 || bg == 31 //左右ドア
          ) {
            ok = true;

            char s[64];
            sprintf( s, "enter bg [%d] %d", bdir, bg );
            TRACE( s );
          }
        }
      }
      break;
  }


  if( ok ) {
    //行ける
    Block* blk = DUNMAP()->getBlock( TOINT(g_plx), TOINT(g_ply) );
    BDIDATA* bdid = blk->getBDIData( bdir ); //BDIR と DIR は同じ扱いで良いはず。(FAR == N, RIGHT == E, NEAR == S, LEFT == W)

    uint8_t a=0, b=0;
    switch( blk->getInfo( bdir ) ) {
      case BDINFO_CORRIDOR: a = bdid->corridor.toArea; b = bdid->corridor.toBlock; break;
      case BDINFO_DOOR: a = bdid->door.toArea; b = bdid->door.toBlock; break;
    }
    
#if defined( DBG_MAP )
    DBGtoarea = a;
    DBGtoblock = b;

    {
      char s[64];
      sprintf( s, "enter [%d_%d] dir:%d  to %d_%d", DUNMAP()->getCurAreaIdx(), blk->getDist(), bdir, a, b );
      TRACE( s );
    }
#endif

    DUNMAP()->enter( a, b );

    return true;
  }
  return false;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void plSetPos( int16_t x, int16_t y )
{
  g_plx = TOFIX(x);
  g_ply = TOFIX(y);
}

static bool plCheckRect( Rect8& crect, int16_t x, int16_t y, Rect8& rect, bool flip )
{
  int16_t sx0, sx1;
  if( flip ) {
    sx0 = x - rect.x;
    sx1 = sx0 - rect.w; 
  } else {
    sx0 = x + rect.x;
    sx1 = sx0 + rect.w;
  }
  if( sx1 < sx0 ) {
    int16_t t = sx0;
    sx0 = sx1;
    sx1 = t;
  }
  int16_t sy0 = y + rect.y;
  int16_t sy1 = sy0 + rect.h;
  
  int16_t dx0, dx1;
  dx0 = TOINT(g_plx);
  if( g_plflip ) {
    dx0 -= crect.x;
    dx1 = dx0 - crect.w;
  } else {
    dx0 += crect.x;
    dx1 = dx0 + crect.w;
  }
  if( dx1 < dx0 ) {
    int16_t t = dx0;
    dx0 = dx1;
    dx1 = t;
  }
  int16_t dy0 = TOINT(g_ply) + crect.y;
  int16_t dy1 = dy0 + crect.h;

  if(sx0 <= dx1 && dx0 <= sx1 && sy0 <= dy1 && dy0 <= sy1) {
    return true;
  }
  return false;
}


bool plCheckMvRect( int16_t x, int16_t y, Rect8& rect, bool flip )
{
  return plCheckRect( g_plmvrect, x, y, rect, flip );
}

bool plCheckDfRect( int16_t x, int16_t y, Rect8& rect, bool flip )
{
  return plCheckRect( g_pldfrect, x, y, rect, flip );
}

ITEM* plGetEqWeapon()
{
  return g_plstat.equip[ EQ_WEAPON ];
}

bool plEquip( ITEM* item )
{
  int8_t eqpos = plCheckEquipPos( item );

  if( eqpos < EQMAX ) {
    //既に装備されていれば出来ない
    if( !g_plstat.equip[eqpos] ) {
      g_plstat.equip[eqpos] = item;
      //パラメータ反映
      applyItem( item );

      return true;
    }
  }

  return false;
}

int8_t plCheckEquipPos( ITEM* item )
{
  int8_t eqpos = EQMAX;

  //item の種類によって装備出来る場所が決まってる
  ITEMBASE* ib = itGetItemBase( item );
  switch( ib->type ) {
    case IT_WEAPON: eqpos = EQ_WEAPON; break;
    case IT_SHIELD: eqpos = EQ_SHIELD; break;
    case IT_HEAD: eqpos = EQ_HEAD; break;
    case IT_ARMOR: eqpos = EQ_BODY; break;
    case IT_RING: eqpos = EQ_RING; break;
    case IT_AMULET: eqpos = EQ_AMULET; break;
    case IT_FOOT: eqpos = EQ_FOOT; break;
    case IT_HAND: eqpos = EQ_HAND; break;
  }

  return eqpos;
}

ITEM* plGetEquipItem( int8_t eqpos )
{
  return g_plstat.equip[ eqpos ];
}

/*
 * 装備から外すだけ。不要なら delete をする事。
 */
ITEM* plUnequip( int8_t eqpos )
{
  ITEM* item = g_plstat.equip[eqpos];

  //パラメータ反映
  if( item ) {
    removeItem( item );
    g_plstat.equip[eqpos] = NULL;
  }

  return item;
}

ITEM* plUnequip( ITEM* item )
{
  for( int8_t i=0; i<EQMAX; i++ ) {
    if( g_plstat.equip[i] == item ) {
      g_plstat.equip[i] = NULL;

      //パラメータ反映
      removeItem( item );
      break;
    }
  }

  return item;
}

/*
 * アイテムの効果反映
 */
void applyItem( ITEM* item )
{
  g_plstat.minatk += item->minatk;
  g_plstat.maxatk += item->maxatk;
  g_plstat.def += item->def;
  g_plstat.hpmax += item->addhpmax;
}

/*
 * アイテムの効果除去
 */
void removeItem( ITEM* item )
{
    //外した装備のパラメータを無くす
    g_plstat.minatk -= item->minatk;
    g_plstat.maxatk -= item->maxatk;
    g_plstat.def -= item->def;
    g_plstat.hpmax -= item->addhpmax;
}

/*
 * アイテムを持たせる
 */
bool plAddItem( ITEM* item )
{
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    if( !plGetStat().items[i] ) {
      plGetStat().items[i] = item;
      return true;
    }
  }
  return false;
}

/*
 * アイテムを捨てる
 * リストからの削除のみでアイテム自体は破棄しない。
 * リストに隙間が出来ない様に削除箇所以降を前に詰めるのもやる。
 */
bool plDelItem( ITEM* item )
{
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    if( plGetStat().items[i] == item ) {
      //後ろを前に詰める
      for( ;i<MAX_PLITEM-1; i++ ) {
        plGetStat().items[i] = plGetStat().items[i+1];
      }
      plGetStat().items[MAX_PLITEM-1] = NULL; //一番最後をクリア

      return true;
    }
  }
  return false;
}

/*
 * アイテムの数を返す
 */
int8_t plGetItemCount()
{
  int8_t ret = 0;
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    if( plGetStat().items[i] ) ret++;
  }

  return ret;
}


/*
 * アイテムを上書き
 * olditem の場所に newitem をセット
 */
void plOverwriteItem( ITEM* olditem, ITEM* newitem )
{
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    if( plGetStat().items[i] == olditem ) {
      plGetStat().items[i] = newitem;
      break;
    }
  }
}

/*
 * 指定の敵から受けるダメージを計算
 */
int16_t plCalcDamage( EnemyData* ed )
{
  /*
   * damage:
   * eatk = (eamin+eamax)/2 + estr*0.5 + (estr*0.5 * elvl)/100
   * 
   * (eatk * eatk) / (eatk + pdef)
   */
  PLSTAT& ps = plGetStat();
  float eatk = (ed->str + (ed->str * ed->lvl)/100.0f);
  float dmg = (eatk * eatk) / (eatk + ps.def);

  return (int16_t)dmg;
}

bool plDamage( EnemyData* ed, int16_t dmg )
{
  g_plstat.hp -= dmg;
  if( g_plstat.hp <= 0 ) {
    g_plstat.hp = 0;
    return true; //死んだ
  }
  return false;
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

