#include "player.h"

#include "pic.h"

#include "dungeon.h"

#include "enemy.h"

#include "item.h"

#include "ui.h"

#include "spatk.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------



//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//fighter
int16_t g_plx, g_ply;
static bool g_plflip;
static uint8_t g_planm;
static uint8_t g_planmwait;
static uint8_t g_plphase;
static uint8_t g_plmode;
static uint8_t g_plwait;
static uint8_t g_plwkflag; //ワーク的に使用するフラグ
static int16_t g_plentermx, g_plentermy;
static uint8_t g_plenterfrm;
static BlockDir g_plenterdir;
Rect8 g_plmvrect;
Rect8 g_platrect;
Rect8 g_pldfrect;
PLSTAT g_plstat;
ITEM* g_plequip[EQMAX];
ITEM* g_plitems[MAX_PLITEM];

//wizard
int16_t g_wzx, g_wzy;
static bool g_wzflip;
static uint8_t g_wzanm;
static uint8_t g_wzanmwait;
static uint8_t g_wzmode;
static uint8_t g_wzphase;
static uint8_t g_wzwait;
static EnemyData* g_wztgted;
static int16_t g_wzcasttgtx, g_wzcasttgty;

//magic: light ball  (effect として分けるかな)
int16_t g_lbx, g_lby, g_lbfy;
int8_t g_lbpow;
uint8_t g_lbanm;
uint8_t g_lbanmwait;
int16_t g_lbang;


enum : uint8_t {
  PLMODE_UNDEFINED,
  PLMODE_MOVE, //移動モード
  PLMODE_ATTACK, //攻撃モード
  PLMODE_ACTION, //行動モード
  PLMODE_ENTER, //マップ切替時自動移動
};

//ワーク的なフラグ。処理中に一時的に必要なフラグ。
enum : uint8_t {
  WF_DONEATTACKCHK = (1 << 0), //攻撃判定を完了した。
};

//fighter phase
enum : uint8_t {
  PLPHASE_MV_WAIT,
  PLPHASE_MV_RIGHT,
  PLPHASE_MV_LEFT,
  PLPHASE_AT_WAIT, //攻撃モード待機
  PLPHASE_AT_ATTACK, //攻撃モード攻撃中（当たり判定発生中)
  PLPHASE_AT_COOLDOWN, //攻撃モード攻撃後硬直
  PLPHASE_AC_WAIT, //行動モード待機
};

//wizard mode
enum : uint8_t {
  WZMODE_UNDEFINED,
  WZMODE_MOVE,
  WZMODE_CAST,
};

//wizard phase
enum : uint8_t {
  WZPHASE_MV_WAIT, //待機中
  WZPHASE_MV_TRACE, //プレイヤーの背後に移動中
  WZPHASE_AT_CAST, //詠唱中
  WZPHASE_AT_COOLDOWN, //詠唱後
};

#if defined( DBG_MAP )
uint8_t DBGtoarea, DBGtoblock;
#endif

//中心位置。左上からのオフセット
#define PLCX 4
#define PLCY 16
#define PLPICW 8
#define PLPICH 16

#define WZCX 6
#define WZCY 16
#define WZPICW 12
#define WZPICH 16

static void wzInit();
static void wzUpdate();
static void wzDraw();
static void wzFinish();
static void wzMoveInit();
static void wzMoveFinish();
static void wzMoveUpdate();
static void wzMoveDraw();
static void wzCastInit();
static void wzCastFinish();
static void wzCastUpdate();
static void wzCastDraw();
static void wzCheckCastSpell();

//light ball
static void lbInit();
static void lbUpdate();
static void lbDraw();
static void lbFinish();


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
static void modeEnter();
static void modeEnterInit();
static void modeEnterFinish();
static void modeEnterDraw();

static void enterMode( uint8_t newmode );
static void enterWzMode( uint8_t newmode );

static void plDrawStat();

static void applyItem(ITEM* item);
//static void removeItem(ITEM* item);
static void calcEquipItem(); //装備中のアイテム効果反映

static void gatherAction( uint8_t slot[UICtrl::BCSLOTMAX] );
static bool checkActionTarget( ObjBase* obj );

static void actStair( bool descend );
static void actGet();
static void actLoot();

static void updateScroll();

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
  g_platrect.w = 5;
  g_platrect.h = 16;

  g_plwkflag = 0;

  memset( &g_plstat, 0, sizeof(PLSTAT) );
  g_plstat.hpmax = g_plstat.hp = 20;
  g_plstat.lvl = 1;
  g_plstat.str = 8;
  g_plstat.dex = 4;
  g_plstat.intl = 4;
  g_plstat.luk = 4;
  g_plstat.curfloor = 1; //1F

  for ( int8_t i = 0; i < EQMAX; i++ ) {
    g_plequip[i] = NULL;
  }
  for ( int8_t i = 0; i < MAX_PLITEM; i++ ) {
    g_plitems[i] = NULL;
  }

  //次のレベルへの経験値を記録しておく（float使ってて遅そうなので）。
  //次に計算するのはレベルアップ時
  g_plstat.nextexp = plCalcExpReqNextLvl( g_plstat.lvl );

  //x!x! 適当な武器を装備させておく
  ITEM* weapon = itGenerate( IBI_SHORTSWORD, ITRANK_NORMAL, 10 );
  plEquip( weapon );
  ITEM* armor = itGenerate( IBI_LEATHERARMOR, ITRANK_NORMAL, 10 );
  plEquip( armor );

  //wizard
  wzInit();

  lbInit();//x!x! test
}

void plUpdate()
{
  if ( g_plmode != PLMODE_ENTER ) { //PLMODE_ENTER 時は自動移動なので操作出来ない
    if ( gamemain.isPress( BUTTON_A ) ) {
      enterMode( PLMODE_ATTACK );
    } else if ( gamemain.isPress( BUTTON_B ) ) {
      enterMode( PLMODE_ACTION );
    } else {
      enterMode( PLMODE_MOVE );
    }
  }

  switch ( g_plmode ) {
    case PLMODE_MOVE: modeMove(); break;
    case PLMODE_ATTACK: modeAttack(); break;
    case PLMODE_ACTION: modeAction(); break;
    case PLMODE_ENTER: modeEnter(); break;
  }


  wzUpdate();

  lbUpdate(); //x!x! test
}

void enterMode( uint8_t newmode )
{
  if ( g_plmode == newmode ) return; //既に同じモード

  //以前のモードの後始末
  switch ( g_plmode ) {
    case PLMODE_MOVE: modeMoveFinish(); break;
    case PLMODE_ATTACK: modeAttackFinish(); break;
    case PLMODE_ACTION: modeActionFinish(); break;
    case PLMODE_ENTER: modeEnterFinish(); break;
  }

  //新しいモードの初期化
  switch ( newmode ) {
    case PLMODE_MOVE: modeMoveInit(); break;
    case PLMODE_ATTACK: modeAttackInit(); break;
    case PLMODE_ACTION: modeActionInit(); break;
    case PLMODE_ENTER: modeEnterInit(); break;
  }

}

//------------------------------------------
void modeMoveInit()
{
  g_planm = 4;
  g_planmwait = 0;
  g_plmode = PLMODE_MOVE;
  g_plphase = PLPHASE_MV_WAIT;
}

void modeMoveFinish()
{
}

void modeMove()
{
  uint8_t plact = g_plphase; //プレイヤーの挙動。待機中、右移動中、左移動中、...
  int16_t cx, cy;
  cx = g_plx;
  cy = g_ply;
  cx += g_plflip ? TOFIX(-PLCX) : TOFIX(PLCX); //判定を行う位置は、画像の進行方向側の端。


  if ( gamemain.isTrigger( BUTTON_UP ) ) {
    if ( enter( BDIR_FAR, cx, cy ) ) return; //奥の入口に入る
  } else if ( gamemain.isTrigger( BUTTON_DOWN ) ) {
    if ( enter( BDIR_NEAR, cx, cy ) ) return; //手前の入口に入る
  }

  int16_t mx = 0;
  int16_t spd = TOFIX(0.75f);
  int16_t mv = 0;

  if ( gamemain.isPress( BUTTON_LEFT ) ) {
    mv = -spd;
    g_plflip = true;
    plact = PLPHASE_MV_LEFT;
  } else if ( gamemain.isPress( BUTTON_RIGHT ) ) {
    mv = spd;
    g_plflip = false;
    plact = PLPHASE_MV_RIGHT;
  } else {
    plact = PLPHASE_MV_WAIT;
  }

  //移動可能チェック
  //とりあえず BG を見る
  cx += mv;

  //エリアの端では LEFT, RIGHT の通路へ出入り
  if ( (plact == PLPHASE_MV_LEFT) && enter( BDIR_LEFT, cx, cy ) ) return;
  if ( (plact == PLPHASE_MV_RIGHT) && enter( BDIR_RIGHT, cx, cy ) ) return;

  //そうで無ければ移動可能か調べる
  uint8_t bgattr = DUNMAP()->getAttrBG( TOINT(cx), TOINT(cy) );
  bool movable = !BGisBlock( bgattr );


  //敵にぶつかるか調べる
  {
    int16_t tx, ty;
    tx = g_plx + mv;
    ty = g_ply;
    EnemyData* ed = enCheckMvRect( TOINT(tx), TOINT(ty), g_plmvrect, g_plflip );
    if ( ed ) {
      movable = false;
    }
  }

  if ( movable ) {
    g_plx += mv;
  }

#define SCROLLMARGIN 24

  //背景スクロール
  updateScroll();

  //animation
  //左右は反転使う？
  if ( plact == g_plphase ) { //行動が変わってない？
    if ( ++g_planmwait == 3 ) {
      switch ( g_plphase ) {
        case PLPHASE_MV_WAIT: g_planm = 4; break; //とりあえずアニメーションしない
        case PLPHASE_MV_RIGHT: g_planm = (g_planm + 1) & 0x3; break;
        case PLPHASE_MV_LEFT: g_planm = (g_planm + 1) & 0x3; break;
      }
      g_planmwait = 0;
    }
  } else {
    //行動が変わった
    g_plphase = plact;
    switch ( g_plphase ) {
      case PLPHASE_MV_WAIT: g_planm = 4; break;
      case PLPHASE_MV_RIGHT: g_planm = 0; break;
      case PLPHASE_MV_LEFT: g_planm = 0; break; //逆は反転で
    }
    g_planmwait = 0;
  }

}

void updateScroll()
{
  int16_t hx, hy;
  DUNMAP()->getHome( hx, hy );
  int16_t scrx = DUNMAP()->toScrX( plGetX() );
  if ( scrx >= (SCRW - SCROLLMARGIN) - 4 ) {
    hx += scrx - ((SCRW - SCROLLMARGIN) - 4 - 1);
    DUNMAP()->setHomeX( hx );
  } else if ( scrx < (SCROLLMARGIN) + 4 ) {
    hx -= (SCROLLMARGIN + 4) - scrx;
    DUNMAP()->setHomeX( hx );
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

  UIC().openBtnCmd( UICtrl::BCMD_A,
                    UICtrl::BCMD_EMPTY,  //up
                    g_plflip ? UICtrl::BCMD_EMPTY : UICtrl::BCMD_ATTACK, //right
                    UICtrl::BCMD_EMPTY, //down
                    g_plflip ? UICtrl::BCMD_ATTACK : UICtrl::BCMD_EMPTY //left
                  );
}

void modeAttackFinish()
{
  UIC().closeBtnCmd();
}

void modeAttack()
{
  uint8_t phase = g_plphase;

  switch ( phase ) {
    case PLPHASE_AT_WAIT:
      {
        if ( (!g_plflip && gamemain.isPress( BUTTON_RIGHT )) ||
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
          if ( !(g_plwkflag & WF_DONEATTACKCHK) ) {
            int16_t tx, ty;
            tx = g_plx;
            ty = g_ply;
            EnemyData* ed = enCheckDfRect( TOINT(tx), TOINT(ty), g_platrect, g_plflip );
            if ( ed ) {
              int16_t dmg = enCalcDamage( ed );
              bool dead = enDamage( ed, dmg );
            }
            g_plwkflag |= WF_DONEATTACKCHK; //判定は攻撃１回につき１回だけ
          }
        }

        //時間が来るまで phase は変わらないが、当たり判定は1回行なったら残り時間では行わない
        if ( --g_plwait == 0 ) {
          g_plwait = 5;
          phase = PLPHASE_AT_COOLDOWN;
        }
      }
      break;
    case PLPHASE_AT_COOLDOWN:
      if ( --g_plwait == 0 ) {
        phase = PLPHASE_AT_WAIT;
      }
      break;
  }

  if ( phase != g_plphase ) {

    g_plphase = phase;
  }

}

//------------------------------------------
void modeActionInit()
{
  g_planm = 4;
  g_planmwait = 0;
  g_plmode = PLMODE_ACTION;
  g_plphase = PLPHASE_AC_WAIT;

  uint8_t slot[UICtrl::BCSLOTMAX ];

  gatherAction( slot );

  UIC().openBtnCmd( UICtrl::BCMD_B, slot[0], slot[1], slot[2], slot[3] );
}

void modeActionFinish()
{
  UIC().closeBtnCmd();
}

void modeAction()
{
  uint8_t slot = UICtrl::BCSLOTMAX;
  if ( gamemain.isTrigger( BUTTON_UP ) ) {
    slot = UICtrl::BCSLOT_UP;
  } else if ( gamemain.isTrigger( BUTTON_RIGHT ) ) {
    slot = UICtrl::BCSLOT_RIGHT;
  } else if ( gamemain.isTrigger( BUTTON_DOWN ) ) {
    slot = UICtrl::BCSLOT_DOWN;
  } else if ( gamemain.isTrigger( BUTTON_LEFT ) ) {
    slot = UICtrl::BCSLOT_LEFT;
  }
  if ( slot != UICtrl::BCSLOTMAX ) {
    uint8_t cmd = UIC().getBtnCmd( slot );
    if ( cmd != UICtrl::BCMD_EMPTY ) {
      switch ( cmd ) {
        case UICtrl::BCMD_UP: actStair( false ); break;
        case UICtrl::BCMD_DOWN: actStair( true ); break;
        case UICtrl::BCMD_GET: actGet(); break;
        case UICtrl::BCMD_LOOT: actLoot(); break;
      }
      //一旦ボタンコマンド解除
      enterMode( PLMODE_MOVE );
    }
  }
}

/*
   自分周辺の B ボタンコマンドに反応する object を調べてコマンド登録
*/
void gatherAction( uint8_t slot[UICtrl::BCSLOTMAX] )
{
  int8_t si = 0;
  Area* area = DUNMAP()->getCurArea();

  uint8_t flag = 0;
  enum {
    FLAG_LOOT = (1 << 0),
    FLAG_UP = (1 << 1),
    FLAG_DOWN = (1 << 2),
    FLAG_GET = (1 << 3),
  };

  for ( int8_t i = 0; i < MAX_OBJECT; i++ ) {
    ObjBase* obj = area->getObj( i );
    if ( obj->isContained() ) continue; //何かに収納されている物は無視

    //当たり判定
    if ( checkActionTarget( obj ) ) {
      switch ( obj->getAction() ) {
        case UICtrl::BCMD_LOOT:
          flag |= FLAG_LOOT;
          break;
        case UICtrl::BCMD_UP:
          flag |= FLAG_UP;
          break;
        case UICtrl::BCMD_DOWN:
          flag |= FLAG_DOWN;
          break;
        case UICtrl::BCMD_GET:
          flag |= FLAG_GET;
          break;
      }
    }
  }

  if ( flag & FLAG_LOOT ) slot[si++] = UICtrl::BCMD_LOOT;
  if ( flag & FLAG_UP ) slot[si++] = UICtrl::BCMD_UP;
  if ( flag & FLAG_DOWN ) slot[si++] = UICtrl::BCMD_DOWN;
  if ( flag & FLAG_GET ) slot[si++] = UICtrl::BCMD_GET;

  for ( ; si < UICtrl::BCSLOTMAX; si++ ) {
    slot[si] = UICtrl::BCMD_EMPTY;
  }
}

/*
   指定 object が操作可能範囲内にいるか返す
*/
bool checkActionTarget( ObjBase* obj )
{
  if ( !obj ) return false;

  int16_t x0, x1;
  int16_t px0, px1;

  px0 = plGetX() - 4;
  px1 = px0 + 8;

  x0 = obj->getX();
  x1 = x0 + obj->getActionRegionW();

  return px0 <= x1 && x0 <= px1;
}

/*
   操作範囲内に有る指定コマンドの対象となる object を集める。
*/
uint8_t collectActTarget( uint8_t cmd, ObjBase** out )
{
  uint8_t cnt = 0;
  Area* area = DUNMAP()->getCurArea();

  for ( int8_t i = 0; i < MAX_OBJECT; i++ ) {
    ObjBase* obj = area->getObj( i );
    if ( obj->isContained() ) continue; //何かに収納されている物は含めない
    if ( checkActionTarget( obj ) ) {
      if ( obj->getAction() == cmd ) {
        out[cnt++] = obj;
      }
    }
  }

  return cnt;
}

void actStair( bool descend )
{
  dunFinish();
  enFinish();

  uint8_t f = plGetFloor();
  if ( descend ) {
    if ( f < 100 ) f++;
  } else {
    if ( f > 1 ) f--;
  }
  plSetFloor( f );

  enInit();
  dunInit();
  DUNMAP()->create( f );

  //プレイヤー配置
  DUNMAP()->enterFloor( descend );
}

void actGet()
{
  uint8_t cnt;
  ObjBase* tgt[MAX_OBJECT]; //１エリア内の最大 object 数分用意する

  cnt = collectActTarget( UICtrl::BCMD_GET, tgt );

  if ( !cnt ) return; //ここに来てる時点で無いはずだけど、１つも無ければ何もしない

  if ( cnt == 1 ) { //１つだけならそのまま拾う
    ObjDropItem* odi = static_cast<ObjDropItem*>( tgt[0] );
    ITEM* item = odi->peekItem();

    if ( plIsItemFull() ) {
      showModalInfoDlg( "Item is full" ); //閉じるまで進行停止する
    } else {
      plAddItem( item );
      odi->detachItem(); //ITEM の割当を解除。解除しないとデストラクタで削除されてしまう。
      DUNMAP()->getCurArea()->removeObj( odi ); //この中で Object が削除される

      char s[21];
      sprintf( s, "Got %s", itGetBaseName(item) );
      ModelessDlgInfo* mdi = showModelessInfo( s, 25 );
      mdi->setFontColor( itGetRankColor( item ) );

    }
  } else { //二つ以上あれば選択画面を出す
    takeObjDropItemMenu( "Take", NULL, cnt, tgt ); //title, parent container, count, list
  }
}

void actLoot()
{
  uint8_t cnt;
  ObjBase* tgt[MAX_OBJECT]; //１エリア内の最大 object 数分用意する

  cnt = collectActTarget( UICtrl::BCMD_LOOT, tgt );

  if ( !cnt ) return; //ここに来てる時点で無いはずだけど、１つも無ければ何もしない

  if ( cnt == 1 ) { //１つだけならそれにアクセス
    ObjContainer* oc = static_cast<ObjContainer*>( tgt[0] );
    takeObjDropItemMenu( "Loot", oc, ObjContainer::MAX_CONTENTS, oc->getContentsList() );
  } else { //二つ以上あればコンテナ選択
    //コンテナが複数ある
    //x!x! コンテナが２個同時にアクセスできる場所に存在する事ある？
    //x!x! 生成時に気をつける？
  }
}


//------------------------------------------
void modeEnterInit()
{
  //#define ENTERV 4 //上下は 4 ずらしてる
#define ENTERV 8 //上下はラインを合わせるために確実に ENTEROFSTV 分移動させる必要がある
#define ENTERH 8 //左右は 8 にしてみる

  //x!x! どうも通路のつながりがおかしくて、ドア同士でつながって無い場合があるもよう。
  //x!x! その場合 frm とか mx, my が初期化されず不定な値でおかしくなってる
  //x!x! つうろのつながりおかしいのなおさないと駄目

  switch ( g_plenterdir ) {
    case BDIR_FAR:
      g_plentermx = 0;
      g_plentermy = TOFIX(0.5); //ENTERV*0.5 で 4
      g_plenterfrm = ENTERV;
      break;
    case BDIR_RIGHT:
      g_plentermx = TOFIX(-0.75);
      g_plentermy = 0;
      g_plenterfrm = ENTERH;
      break;
    case BDIR_NEAR:
      g_plentermx = 0;
      g_plentermy = TOFIX(-0.5); //ENTERV*0.5 で 4
      g_plenterfrm = ENTERV;
      break;
    case BDIR_LEFT:
      g_plentermx = TOFIX(0.75);
      g_plentermy = 0;
      g_plenterfrm = ENTERH;
      break;
  }
  g_planm = 0;
  g_planmwait = 1; //アニメーションがすぐ起こる様にすすめておく
  g_plmode = PLMODE_ENTER;
}

void modeEnterFinish()
{
}

void modeEnter()
{
  g_plx += g_plentermx;
  g_ply += g_plentermy;
  if ( g_plentermx ) {
    g_plflip = (g_plentermx < 0) ? true : false;
  }
  if ( --g_plenterfrm == 0 ) {
    enterMode( PLMODE_MOVE );
  }

  //animation
  //左右は反転使う？
  if ( ++g_planmwait == 3 ) {
    if ( g_plentermx >= 0 ) g_planm = (g_planm + 1) & 0x3;
    if ( g_plentermx < 0) g_planm = (g_planm + 1) & 0x3;
    g_planmwait = 0;
  }

  updateScroll();
}

void modeEnterDraw()
{
  modeMoveDraw();
}


//------------------------------------------
void plDraw()
{
  wzDraw(); //wizard の上に fighter が描画される様に。

  switch ( g_plmode ) {
    case PLMODE_MOVE: modeMoveDraw(); break;
    case PLMODE_ATTACK: modeAttackDraw(); break;
    case PLMODE_ACTION: modeActionDraw(); break;
    case PLMODE_ENTER: modeEnterDraw(); break;
  }

  lbDraw(); //x!x! test  fighter の上に

  plDrawStat();


#if defined( DBG_SHOW_HITRECT )
  {
    int16_t x, y, rx, ry;
    x = DUNMAP()->toScrX(TOINT(g_plx));
    y = DUNMAP()->toScrY(TOINT(g_ply));
    //move(green)
    if ( g_plflip ) {
      rx = x - g_plmvrect.x - g_plmvrect.w;
    } else {
      rx = x + g_plmvrect.x;
    }
    ry = y + g_plmvrect.y;
    gb.display.setColor( Color::lightgreen );
    gb.display.drawRect( rx, ry, g_plmvrect.w, g_plmvrect.h );
    //attack(red)
    if ( g_plflip ) {
      rx = x - g_platrect.x - g_platrect.w;
    } else {
      rx = x + g_platrect.x;
    }
    ry = y + g_platrect.y;
    gb.display.setColor( Color::red );
    gb.display.drawRect( rx, ry, g_platrect.w, g_platrect.h );
    //defense(blue)
    rx = x + g_pldfrect.x;
    if ( g_plflip ) {
      rx = x - g_pldfrect.x - g_pldfrect.w;
    } else {
      rx = x + g_pldfrect.x;
    }
    ry = y + g_pldfrect.y;
    gb.display.setColor( Color::lightblue );
    gb.display.drawRect( rx, ry, g_pldfrect.w, g_pldfrect.h );
  }
#endif

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
  gb.display.drawImage( x - PLCX, y - PLCY, *getPic( PIC_CHARACTER ), g_plflip ? -PLPICW : PLPICW, PLPICH );

  //  FBL().setLight( DUNMAP()->toScrX(plGetX()), DUNMAP()->toScrY(plGetY())-8, 28 );

  //武器があったら描画
  ITEM* weapon = plGetEqWeapon();
  if ( weapon ) {
    ITEMBASE* ib = itGetItemBase( weapon->base );
    ItemPic* ip = ib->eqpic;
    if ( g_plflip ) {
      x -= wppnt[ frm * 2 + 0 ];
      lx = x;
      x -= (ip->w + ip->x);
    } else {
      x += wppnt[ frm * 2 + 0 ];
      lx = x;
      x += ip->x;
    }
    y += wppnt[ frm * 2 + 1 ] + ip->y;
    ly = y - ip->y;
    ip->pic->setFrame( ip->frm );
    gb.display.drawImage( x, y - PLCY, *(ip->pic), g_plflip ? -ip->w : ip->w, ip->h );

    //    FBL().setLight( lx, ly-PLCY, 20, fbIllumination::LVL_8 );
  }

#if 1
  {
#if defined( DBG_MAP )
    {
      char s[64];
      Area* ca = DUNMAP()->getCurArea();
      sprintf( s, PSTR("%d,%d->%d"), ca->DBGgetSX(), ca->DBGgetSY(), ca->DBGgetDir() );
      gb.display.setCursor( 40, 6 );
      gb.display.print( s );
    }
#endif

  }
#endif
}

void modeAttackDraw()
{
  int16_t frm = 4;

  switch ( g_plphase ) {
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
  gb.display.drawImage( x - PLCX, y - PLCY, *getPic( PIC_CHARACTER ), g_plflip ? -PLPICW : PLPICW, PLPICH );

  //武器があったら描画
  ITEM* weapon = plGetEqWeapon();
  if ( weapon ) {
    ITEMBASE* ib = itGetItemBase( weapon->base );

    static const int8_t wppnt[] = { //各フレームにおける武器配置位置 offset (x の中央(-PLCX前の値)からのオフセット)
      //x(横幅の中央からのオフセット),y(最上部からのオフセット)
      0, 7, //frm 4
      4, 6, //frm 5
    };
    frm -= 4;
    ItemPic* ip = ib->eqpic;
    if ( g_plflip ) {
      x -= wppnt[ frm * 2 + 0 ];
      lx = x;
      x -= (ip->w + ip->x);
    } else {
      x += wppnt[ frm * 2 + 0 ];
      lx = x;
      x += ip->x;
    }
    y += wppnt[ frm * 2 + 1 ] + ip->y;
    ly = y - ip->y;
    ip->pic->setFrame( ip->frm );
    gb.display.drawImage( x, y - PLCY, *(ip->pic), g_plflip ? -ip->w : ip->w, ip->h );


    //    FBL().setLight( lx, ly-PLCY, 20, fbIllumination::LVL_8 );
  }

}

void modeActionDraw()
{
  modeMoveDraw();
}

void plDrawStat()
{
  //HP
  getPic( PIC_ICON6x5 )->setFrame( UICtrl::ICON_HEART ); //HEART
  gb.display.drawImage( 0, 0, *getPic( PIC_ICON6x5 ) );
  gb.display.setColor( Color::white );
  gb.display.setCursor( 6, 0 );
  char s[32];
  sprintf( s, PSTR("%d/%d"), g_plstat.hp, g_plstat.hpmax );
  gb.display.print( s );

  //Floor
  gb.display.setColor( Color::white );
  gb.display.setCursor( 0, 6 );
  sprintf( s, PSTR("B%dF"), g_plstat.curfloor );
  gb.display.print(s);



#if !defined( DBG_MAP )
  gb.display.setColor( Color::green );
  gb.display.drawFastHLine( 0, 15, SCRW );
#endif
}

void plFinish()
{
  for ( int8_t i = 0; i < EQMAX; i++ ) {
    if ( g_plequip[i] ) {
      delete g_plequip[i];
      g_plequip[i] = NULL;
    }
  }
  for ( int8_t i = 0; i < MAX_PLITEM; i++ ) {
    if ( g_plitems[i] ) {
      delete g_plitems[i];
      g_plitems[i] = NULL;
    }
  }

  wzFinish();
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

  switch ( bdir ) {
    case BDIR_FAR:  //上下の出入り
    case BDIR_NEAR:
      {
        cy += getDirY( bdir ) * TILEH;
#if 0
        uint8_t bg = DUNMAP()->getMapBG( cx, cy );
        if ( bg == 4 || bg == 7 || bg == 0
             || bg == 19 || bg == 21 //奥のドア
             || bg == 23 || bg == 24 //手前のドア
           ) {
          ok = true;
        }
#else
        uint8_t bgattr = DUNMAP()->getAttrBG( cx, cy );
        ok = BGisEnter( bgattr );
#endif
      }
      break;
    case BDIR_LEFT: //左右の出入り
    case BDIR_RIGHT:
      {
#if 0
        if ( ((TOINT(g_plx) - 4) < 0) || ((TOINT(g_plx) + 4) >= (DUNMAP()->getCurArea()->getWidth())) ) {
          char s[64];
          sprintf( s, "enter pos [%d] %d  %d", bdir, g_plx, DUNMAP()->getCurArea()->getWidth() );
          TRACE( s );

          ok = true;
        } else {
#if 0
          uint8_t bg = DUNMAP()->getMapBG( cx, cy );
          if ( bg == 28 || bg == 31 //左右ドア
             ) {
            ok = true;

            char s[64];
            sprintf( s, "enter bg [%d] %d", bdir, bg );
            TRACE( s );
          }
#else
          uint8_t bgattr = DUNMAP()->getAttrBG( cx, cy );
          ok = BGisEnter( bgattr );
          if( ok ) {
            char s[64];
            sprintf( s, "enter bg [%d] %d", bdir, DUNMAP()->getMapBG(cx,cy) );
            TRACE( s );
          }
#endif
        }
#else
        uint8_t bgattr = DUNMAP()->getAttrBG( cx, cy );
        ok = BGisEnter( bgattr );
        if( ok ) {
          char s[64];
          sprintf( s, "enter bg [%d] %d", bdir, DUNMAP()->getMapBG(cx,cy) );
          TRACE( s );
        }
#endif
      }
      break;
  }


  if ( ok ) {
    //行ける
    Block* blk = DUNMAP()->getBlock( TOINT(g_plx), TOINT(g_ply) );
    BDIDATA* bdid = blk->getBDIData( bdir ); //BDIR と DIR は同じ扱いで良いはず。(FAR == N, RIGHT == E, NEAR == S, LEFT == W)

    uint8_t a = 0, b = 0;
    switch ( blk->getInfo( bdir ) ) {
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

    g_plenterdir = DUNMAP()->enter(
                     a, b, //行き先の area, blk
                     DUNMAP()->getCurAreaIdx(), blk->getDist() //今いる area, blk
                   );
    enterMode( PLMODE_ENTER );

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

void wzSetPos( int16_t x, int16_t y )
{
  g_wzx = TOFIX(x);
  g_wzy = TOFIX(y);
}

void plSetEnterPos( int16_t x, int16_t y )
{
  plSetPos( x, y );
  wzSetPos( x, y );

  //x!x! test
  g_lbx = TOFIX(x);
  g_lby = TOFIX(y - 8);
}

static bool plCheckRect( Rect8& crect, int16_t x, int16_t y, Rect8& rect, bool flip )
{
  int16_t sx0, sx1;
  if ( flip ) {
    sx0 = x - rect.x;
    sx1 = sx0 - rect.w;
  } else {
    sx0 = x + rect.x;
    sx1 = sx0 + rect.w;
  }
  if ( sx1 < sx0 ) {
    int16_t t = sx0;
    sx0 = sx1;
    sx1 = t;
  }
  int16_t sy0 = y + rect.y;
  int16_t sy1 = sy0 + rect.h;

  int16_t dx0, dx1;
  dx0 = TOINT(g_plx);
  if ( g_plflip ) {
    dx0 -= crect.x;
    dx1 = dx0 - crect.w;
  } else {
    dx0 += crect.x;
    dx1 = dx0 + crect.w;
  }
  if ( dx1 < dx0 ) {
    int16_t t = dx0;
    dx0 = dx1;
    dx1 = t;
  }
  int16_t dy0 = TOINT(g_ply) + crect.y;
  int16_t dy1 = dy0 + crect.h;

  if (sx0 <= dx1 && dx0 <= sx1 && sy0 <= dy1 && dy0 <= sy1) {
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
  return g_plequip[ EQ_WEAPON ];
}

bool plEquip( ITEM* item )
{
  int8_t eqpos = plCheckEquipPos( item );

  if ( eqpos < EQMAX ) {
    //既に装備されていれば出来ない
    if ( !g_plequip[eqpos] ) {
      g_plequip[eqpos] = item;
      //パラメータ反映
      calcEquipItem();

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
  switch ( ib->type ) {
    case IT_WEAPON: eqpos = EQ_WEAPON; break;
    case IT_SHIELD: eqpos = EQ_SHIELD; break;
    case IT_HEAD: eqpos = EQ_HEAD; break;
    case IT_ARMOR: eqpos = EQ_BODY; break;
    case IT_RING: eqpos = EQ_RING; break;
    case IT_AMULET: eqpos = EQ_AMULET; break;
    case IT_FEET: eqpos = EQ_FEET; break;
    case IT_HAND: eqpos = EQ_HAND; break;
  }

  return eqpos;
}

ITEM* plGetEquipItem( int8_t eqpos )
{
  return g_plequip[ eqpos ];
}

/*
   装備から外すだけ。不要なら delete をする事。
*/
ITEM* plUnequip( int8_t eqpos )
{
  ITEM* item = g_plequip[eqpos];

  //パラメータ反映
  if ( item ) {
//    removeItem( item );
    g_plequip[eqpos] = NULL;
    calcEquipItem();
  }

  return item;
}

ITEM* plUnequip( ITEM* item )
{
  for ( int8_t i = 0; i < EQMAX; i++ ) {
    if ( g_plequip[i] == item ) {
      g_plequip[i] = NULL;

      //パラメータ反映
//      removeItem( item );
      calcEquipItem();
      break;
    }
  }

  return item;
}

/*
   アイテムの効果反映
*/
void applyItem( ITEM* item )
{
  PLITEMBUFF* pib = &g_plstat.itbuff;
  //パラメータをbyte,wordの配列に分けた方が良いかも…
  pib->minatk += item->minatk;
  pib->maxatk += item->maxatk;
  pib->addhpmax += item->addhpmax;
  pib->addstr += item->addstr;
  pib->adddex += item->adddex;
  pib->addint += item->addint;
  pib->addluk += item->addluk;
  pib->def += item->def;
  pib->healhp += item->healhp;
  pib->registfire += item->registfire;
  pib->registcold += item->registcold;
  pib->registphysic += item->registphysic;
  pib->registthunder += item->registthunder;
  pib->registmagic += item->registmagic;
  pib->itemattr = ItemAttribute( pib->itemattr | item->attr );

  if( pib->minatk >= MAX_PL_MINATK ) pib->minatk = MAX_PL_MINATK;
  if( pib->maxatk >= MAX_PL_MAXATK ) pib->minatk = MAX_PL_MAXATK;
  if( pib->addhpmax >= MAX_PL_ADDHPMAX ) pib->addhpmax = MAX_PL_ADDHPMAX;
  if( pib->addstr >= MAX_PL_ADDSTR ) pib->addstr = MAX_PL_ADDSTR;
  if( pib->adddex >= MAX_PL_ADDDEX ) pib->adddex = MAX_PL_ADDDEX;
  if( pib->addint >= MAX_PL_ADDINT ) pib->addint = MAX_PL_ADDINT;
  if( pib->addluk >= MAX_PL_ADDLUK ) pib->addluk = MAX_PL_ADDLUK;
  if( pib->def >= MAX_PL_DEF ) pib->def = MAX_PL_DEF;
  if( pib->healhp >= MAX_PL_HEALHP ) pib->healhp = MAX_PL_HEALHP;
  if( pib->registfire >= MAX_PL_RGFIRE ) pib->registfire = MAX_PL_RGFIRE;
  if( pib->registcold >= MAX_PL_RGCOLD ) pib->registcold = MAX_PL_RGCOLD;
  if( pib->registphysic >= MAX_PL_RGPHYSIC ) pib->registphysic = MAX_PL_RGPHYSIC;
  if( pib->registthunder >= MAX_PL_RGTHUNDER ) pib->registthunder = MAX_PL_RGTHUNDER;
  if( pib->registmagic >= MAX_PL_RGMAGIC ) pib->registmagic = MAX_PL_RGMAGIC;
}

void calcEquipItem()
{
  //一旦全部クリア
  memset( &g_plstat.itbuff, 0, sizeof(g_plstat.itbuff) );
  for ( int8_t i = 0; i < EQMAX; i++ ) {
    if ( g_plequip[i] ) {
      //パラメータ反映
      applyItem( g_plequip[i] );
    }
  }
}


/*
   アイテムを持たせる
*/
bool plAddItem( ITEM* item )
{
  for ( int8_t i = 0; i < MAX_PLITEM; i++ ) {
    if ( !g_plitems[i] ) {
      g_plitems[i] = item;
      return true;
    }
  }
  return false;
}

/*
   アイテムを捨てる
   リストからの削除のみでアイテム自体は破棄しない。
   リストに隙間が出来ない様に削除箇所以降を前に詰めるのもやる。
*/
bool plDelItem( ITEM* item )
{
  for ( int8_t i = 0; i < MAX_PLITEM; i++ ) {
    if ( g_plitems[i] == item ) {
      //後ろを前に詰める
      for ( ; i < MAX_PLITEM - 1; i++ ) {
        g_plitems[i] = g_plitems[i + 1];
      }
      g_plitems[MAX_PLITEM - 1] = NULL; //一番最後をクリア

      return true;
    }
  }
  return false;
}

/*
   アイテムの数を返す
*/
int8_t plGetItemCount()
{
  int8_t ret = 0;
  for ( int8_t i = 0; i < MAX_PLITEM; i++ ) {
    if ( g_plitems[i] ) ret++;
  }

  return ret;
}


/*
   アイテムを上書き
   olditem の場所に newitem をセット
*/
void plOverwriteItem( ITEM* olditem, ITEM* newitem )
{
  for ( int8_t i = 0; i < MAX_PLITEM; i++ ) {
    if ( g_plitems[i] == olditem ) {
      g_plitems[i] = newitem;
      break;
    }
  }
}

/*
   指定の敵から受けるダメージを計算
*/
int16_t plCalcDamage( EnemyData* ed )
{
  /*
     damage:
     eatk = (eamin+eamax)/2 + estr*0.5 + (estr*0.5 * elvl)/100

     (eatk * eatk) / (eatk + pdef)
  */
  PLSTAT& ps = plGetStat();
  float eatk = (ed->str + (ed->str * ed->lvl) / 100.0f);
  float dmg = (eatk * eatk) / (eatk + plGetDEF());

  return (int16_t)dmg;
}

bool plDamage( int16_t dmg )
{
  g_plstat.hp -= dmg;
  if ( g_plstat.hp <= 0 ) {
    g_plstat.hp = 0;
    return true; //死んだ
  }
  return false;
}

void plAddExp( uint16_t getexp )
{
  g_plstat.ex += getexp;

  while( g_plstat.ex >= g_plstat.nextexp ) {
    g_plstat.ex -= g_plstat.nextexp;
    plLevelUp();
    g_plstat.nextexp = plCalcExpReqNextLvl( g_plstat.lvl );
  }
}

void plLevelUp()
{
  if( ++g_plstat.lvl > MAX_PL_LVL ) g_plstat.lvl = MAX_PL_LVL;

  g_plstat.str++;
  g_plstat.dex++;
  g_plstat.intl++;
  g_plstat.luk++;

  g_plstat.hpmax += 5 + random( g_plstat.luk/10 ) + (g_plstat.lvl/3);
  g_plstat.hp = g_plstat.hpmax;
}

int16_t plCalcExpReqNextLvl( uint8_t curlvl )
{
/*
  次のレベルまでの必要値
    A.-------
    今のレベル n
    (前のレベルでの必要値 * 1.3) * 0.78 + (n*10)*0.22
    LVL1 の時は前のレベルの値がないので固定値。
    LVL1 の固定値 8 の場合、LVL99 での値は 18057。
    99 を超えてレベルが上ってくの向き？
    B.-------
    今のレベル n
    (前のレベルでの必要値 * 1.3) * 0.795 + (n*10)*0.205
    LVL1 の時は前のレベルの値がないので固定値。
    LVL1 の固定値 8 の場合、LVL90 で 29103
    LVL91 以降は最大値固定になる
    99 までの場合向き？
    C.-------
    今のレベル n
    (前のレベルでの必要値 * 1.25) * 0.795 + (n*35)*0.205
    LVL1 の時は前のレベルの値がないので固定値。
    LVL1 の固定値 8 の場合、LVL99 で 29208
 */
 
  float v = 8.0f; //level1 の時の次への必要値

  for( int16_t i=2; i<=curlvl; i++ ) { //level1 は定数なので、1の場合から計算
    v = ((v * 1.25f) * 0.795) + ((i*35.0f)*0.205f);
  }

  int16_t ret = (uint16_t)v;

  if( ret > MAX_PL_EXP ) ret = MAX_PL_EXP;

  return ret;
}

//--------------------------------------------------------------------------
//[wizard]------------------------------------------------------------------
//--------------------------------------------------------------------------
void wzInit()
{
  g_wzx = g_plx;
  g_wzy = g_ply;
  g_wzflip = false;
  g_wzmode = WZMODE_UNDEFINED;
  enterWzMode( WZMODE_MOVE );
//  g_wzphase = WZPHASE_MV_WAIT;
}

void wzUpdate()
{
  switch( g_wzmode ) {
    case WZMODE_MOVE: wzMoveUpdate(); break;
    case WZMODE_CAST: wzCastUpdate(); break;
  }
}

void wzDraw()
{
  switch( g_wzmode ) {
    case WZMODE_MOVE: wzMoveDraw(); break;
    case WZMODE_CAST: wzCastDraw(); break;
  }
}

void wzFinish()
{
}

//--------------------------------------------------------------------------
void enterWzMode( uint8_t newmode )
{
  if ( g_wzmode == newmode ) return; //既に同じモード

  //以前のモードの後始末
  switch ( g_wzmode ) {
    case WZMODE_MOVE: wzMoveFinish(); break;
    case WZMODE_CAST: wzCastFinish(); break;
  }

  //新しいモードの初期化
  switch ( newmode ) {
    case WZMODE_MOVE: wzMoveInit(); break;
    case WZMODE_CAST: wzCastInit(); break;
  }
}

//--------------------------------------------------------------------------
void wzMoveInit()
{
  g_wzanm = 4;
  g_wzanmwait = 0;
  g_wzmode = WZMODE_MOVE;
  g_wzphase = WZPHASE_MV_WAIT;
}

void wzMoveFinish()
{
}

void wzMoveUpdate()
{
  int16_t tgtx, tgty;
  int16_t my = 0, mx = 0;

  tgtx = g_plx + (g_plflip ? TOFIX(8 + PLPICW) : TOFIX(-(8 + PLPICW)));
  tgty = g_ply;

  int16_t aw = TOFIX(DUNMAP()->getCurArea()->getWidth()-WZPICW/2);
  if( tgtx < TOFIX(WZPICW/2) ) {
    tgtx = TOFIX(WZPICW/2);
  } else
  if( tgtx > aw ) {
    tgtx = aw;
  }

  //Y 方向は確実に合わせる
  if ( tgty != g_wzy ) my = g_plentermy;

  //y 移動中は横移動しない
  if ( !my ) {
    int16_t d = tgtx - g_wzx;
    bool trace = false;
    d = TOINT(d);
    if ( d < 0 ) d = -d;
    //目的地と一定以上離れたら移動開始
    switch ( g_wzphase ) {
      case WZPHASE_MV_WAIT:
        if ( d > 6 ) trace = true; //停止中は結構離れるまで待つ
        break;
      case WZPHASE_MV_TRACE: //移動中はかなり近づくまで移動する
        if ( d > 1 ) trace = true;
        break;
    }
    if ( trace ) {
      if ( tgtx < g_wzx ) {
        mx = TOFIX(-0.75f);
        g_wzflip = true;
      }
      if ( tgtx > g_wzx ) {
        mx = TOFIX( 0.75f);
        g_wzflip = false;
      }
    }
  }

  g_wzx += mx;
  g_wzy += my;

  //animation
  if ( mx || my ) {
    if ( ++g_wzanmwait == 3 ) {
      g_wzanm = (g_wzanm + 1) & 0x3;
      g_wzanmwait = 0;
    }
    g_wzphase = WZPHASE_MV_TRACE;
  } else {
    g_wzanm = 4;
    g_wzanmwait = 0;
    g_wzflip = g_plflip; //fighter と同じ方向を向けてみる
    g_wzphase = WZPHASE_MV_WAIT;
  }

  //呪文詠唱判定
  wzCheckCastSpell();
  
}

void wzMoveDraw()
{
  static const uint8_t frmtbl[] = { 1, 2, 3, 2, 0 };

  int16_t x, y;
  int16_t lx, ly;
  x = DUNMAP()->toScrX(TOINT(g_wzx));
  y = DUNMAP()->toScrY(TOINT(g_wzy));

  uint8_t frm = frmtbl[g_wzanm];

  getPic( PIC_WIZARD )->setFrame( frm );
  gb.display.drawImage( x - WZCX, y - WZCY, *getPic( PIC_WIZARD ), g_wzflip ? -WZPICW : WZPICW, WZPICH );

  //光る
//  FBL().setLight( x, y - 10, 16 );

}


void wzCheckCastSpell()
{
  //停止中に呪文詠唱判定
  if( g_wzphase == WZPHASE_MV_WAIT ) {
    //敵が一定範囲に来たら
    EnemyData* ed = enGetInRange( TOINT(g_wzx), TOINT(g_wzy), 40 );

    //x!x! ↓みたいな条件つける？
    //x!x! ・敵がこちらに気づいてなければ攻撃しない
    //x!x! ・気づいて無くても戦士が攻撃モードにしたら攻撃する
    
    if( ed ) {
      g_wzcasttgtx = ed->x;
      g_wzcasttgty = ed->y;
      g_wztgted = ed;

      if( ed->x < g_wzx ) g_wzflip = true;
      else g_wzflip = false;

      EnemyTemplate* et = ENTPL( ed->type );
      g_wzcasttgty -= TOFIX( et->dfrect.h/2 );
          
          
      
      enterWzMode( WZMODE_CAST );
    }
  }
}



//--------------------------------------------------------------------------
void wzCastInit()
{
  g_wzmode = WZMODE_CAST;
  g_wzphase = WZPHASE_AT_CAST;
  g_wzwait = 25; //詠唱時間。呪文によって変える？
}

void wzCastFinish()
{
}

void wzCastUpdate()
{
  switch( g_wzphase ) {
    case WZPHASE_AT_CAST:
      {
        if( --g_wzwait == 0 ) {
          //呪文完成
          SPAC().createFireBall( SpAtCtrl::FLAG_TGT_ENEMY, g_wzx + (g_wzflip ? TOFIX(-6) : TOFIX(6)), g_wzy - TOFIX(8), g_wzcasttgtx, g_wzcasttgty, 4+random(8) );
          g_wzwait = 25; //詠唱後硬直時間。呪文によって変える？
          g_wzphase = WZPHASE_AT_COOLDOWN;
        }
      }
      break;
    case WZPHASE_AT_COOLDOWN:
      {
        if( --g_wzwait == 0 ) {
          enterWzMode( WZMODE_MOVE );
        }
      }
      break;
  }
}

void wzCastDraw()
{
  int16_t x, y;
  x = DUNMAP()->toScrX(TOINT(g_wzx));
  y = DUNMAP()->toScrY(TOINT(g_wzy));
  
  getPic( PIC_WIZARD )->setFrame( 4 );
  gb.display.drawImage( x - WZCX, y - WZCY, *getPic( PIC_WIZARD ), g_wzflip ? -WZPICW : WZPICW, WZPICH );
}

//--------------------------------------------------------------------------
//[magic]-------------------------------------------------------------------
//--------------------------------------------------------------------------
void lbInit()
{
  g_lbanm = 0;
  g_lbanmwait = 3;
  g_lbang = 0;
  g_lbfy = 0;
}

void lbUpdate()
{
  int16_t tgtx, tgty;
  int16_t my = 0, mx = 0;

  //揺れ
  g_lbfy = -FIXSIN( TOINT( g_lbang ) );
  g_lbfy *= 3;
  g_lbang += TOFIX(8.0f);
  if ( g_lbang >= TOFIX(360.0f) ) g_lbang -= TOFIX(360.0f);

  //x!x! 進行方向の先とプレイヤーの位置辺りを行ったり来たりさせる？
  tgtx = g_plx + (g_plflip ? TOFIX(-(30)) : TOFIX(30));
  tgty = g_ply - TOFIX(6);

  //Y 方向は確実に合わせる
  int16_t d = tgty - g_lby;
  d = TOINT(d);
  if( d < 0 ) d = -d;
  if( d != 0 ) {
    d = TOFIX(d) / 5;
    if( d > TOFIX(1.0f) ) d = TOFIX(1.0f);
    if ( tgty < g_lby ) {
      my = -d;
    }
    if ( tgty > g_lby ) {
      my = d;
    }
  }

  //X
  d = tgtx - g_lbx;
  d = TOINT(d);
  if ( d < 0 ) d = -d;
  //目的地と一定以上離れたら移動開始
  if ( d > 6 ) {
    d = TOFIX(d) / 22;
    if( d > TOFIX(1.5f) ) d = TOFIX(1.5f);
    
    if ( tgtx < g_lbx ) {
      mx = -d;
    }
    if ( tgtx > g_lbx ) {
      mx = d;
    }
  }

  int16_t cx = g_lbx + mx;
  int16_t aw = TOFIX(DUNMAP()->getCurArea()->getWidth()-4);
  if( cx < TOFIX(4) ) {
    cx = TOFIX(4);
  } else
  if( cx > aw ) {
    cx = aw;
  }
  g_lbx = cx;
  g_lby += my;
}

void lbDraw()
{
  static const uint8_t frmtbl[] = { 0, 1 };

  int16_t x, y;
  x = DUNMAP()->toScrX(TOINT(g_lbx)) - 4;
  y = DUNMAP()->toScrY(TOINT(g_lby + g_lbfy)) - 4;

  uint8_t frm = frmtbl[g_lbanm];
  if ( --g_lbanmwait == 0 ) {
    g_lbanm ^= 1;
    g_lbanmwait = 3;
  }


  getPic( PIC_EFFECT8x8 )->setFrame( frm );
  gb.display.drawImage( x, y, *getPic( PIC_EFFECT8x8 ) );

  //光る
  FBL().setLight( x, y, g_lbanm ? 16 : 20 );
}

void lbFinish()
{
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
int16_t plGetHPMAX()
{
  int16_t ret = g_plstat.hpmax;
  ret += g_plstat.itbuff.addhpmax;
  if( ret > MAX_PL_HPMAX ) ret = MAX_PL_HPMAX;
  return ret;
}
int16_t plGetMPMAX()
{
  int16_t ret = g_plstat.mpmax;
  ret += g_plstat.itbuff.addmpmax;
  if( ret > MAX_PL_MPMAX ) ret = MAX_PL_MPMAX;
  return ret;
}
int16_t plGetSTR()
{
  int16_t ret = g_plstat.str;
  ret += g_plstat.itbuff.addstr;
  if( ret > MAX_PL_STR ) ret = MAX_PL_STR;
  return ret;
}
int16_t plGetDEX()
{
  int16_t ret = g_plstat.dex;
  ret += g_plstat.itbuff.adddex;
  if( ret > MAX_PL_DEX ) ret = MAX_PL_DEX;
  return ret;
}
int16_t plGetINT()
{
  int16_t ret = g_plstat.intl;
  ret += g_plstat.itbuff.addint;
  if( ret > MAX_PL_INT ) ret = MAX_PL_INT;
  return ret;
}
int16_t plGetLUK()
{
  int16_t ret = g_plstat.luk;
  ret += g_plstat.itbuff.addluk;
  if( ret > MAX_PL_LUK ) ret = MAX_PL_LUK;
  return ret;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void plSave()
{
  //status
  gb.save.set( SDS_PLSTAT, &g_plstat, sizeof(g_plstat) );

  //item
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    itSave( SDS_PLITEM_TOP+i, g_plitems[i] );
  }

  //equip
  for( int8_t i=0; i<EQMAX; i++ ) {
    itSave( SDS_PLEQUIP_TOP+i, g_plequip[i] );
  }
}

void plLoad()
{
  gb.save.get( SDS_PLSTAT, &g_plstat, sizeof(g_plstat) );

  //item
  for( int8_t i=0; i<MAX_PLITEM; i++ ) {
    g_plitems[i] = itLoad( SDS_PLITEM_TOP+i );
  }
  //equip
  for( int8_t i=0; i<EQMAX; i++ ) {
    g_plequip[i] = itLoad( SDS_PLEQUIP_TOP+i );
  }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

