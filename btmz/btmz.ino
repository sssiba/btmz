/*
*/
#include "btmz.h"
#include "gamemain.h"
//#include "pic.h"

#include "player.h"
#include "enemy.h"
#include "dungeon.h"
#include "item.h"

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

//インスタンスが必要
GameMain gamemain;
fbIllumination fbl;


//方向ベクトル
const int8_t g_dirstepx[DIRMAX] = {
  0, 1, 0, -1
};
const int8_t g_dirstepy[DIRMAX] = {
  -1, 0, 1, 0
};

#if defined( DBG_SHOW_FPS )
uint32_t g_prvms;
uint8_t g_updcnt;
uint8_t g_fps;
#endif


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
void setup() {
  // put your setup code here, to run once:
  gb.begin();

  gb.setFrameRate( FRAMERATE );

  gamemain.setup();

  FBL().reset();
  FBL().setClipArea( 0, 16, 79, 63 ); //画面上部はステータス的な所。マップじゃないので適用しない。

#if defined( DBG_SHOW_FPS )
  g_prvms = millis();
  g_updcnt = 0;
  g_fps = 0;
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
  while ( !gb.update() );

  //クリア
  //clear display
  gb.display.clear();
  //clear back light
  //  gb.lights.clear();



  //入力
  gamemain.checkButton();
  //更新
  gamemain.update();
  //描画
  gamemain.draw();

#if defined( DBG_SHOW_FPS )
  { //FPS
    g_updcnt++;
    uint32_t cms = millis();
    if ( (cms - g_prvms) >= 1000 ) {
      g_prvms = cms;
      g_fps = g_updcnt;
      g_updcnt = 0;
    }
    gb.display.setColor( ColorIndex::gray );
    gb.display.setCursor( 0, 58 );
    char s[16];
    sprintf( s, PSTR("%d"), g_fps );
    gb.display.print( s );
  }
#endif
}

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
enum {
  PHASE_NONE,
  PHASE_GAME,
  PHASE_MENU,
  PHASE_MENU_ITEM,
  PHASE_MENU_EQUIP,
  PHASE_MENU_STATUS,
  PHASE_MENU_OBJDROPITEM,
};
static uint8_t g_phase;
static uint8_t g_nextphase;
static WinSelect* g_win;
static DlgInfo* g_dlginfo;

static void enterMainMenu();
static bool updateMainMenu();

static bool updateMenuStatus();
static void drawMenuStatus();

static bool initMenuItem();
static bool updateMenuItem();
static void drawMenuItem();
static void finishMenuItem();

static void initMenuEquip();
static bool updateMenuEquip();
static void drawMenuEquip();
static void finishMenuEquip();

static void initMenuObjDropItem();
static bool updateMenuObjDropItem();
static void drawMenuObjDropItem();
static void finishMenuObjDropItem();

static const int8_t FONTW = 4;
static const int8_t FONTH = 6;

static void changePhase();

//-------------------------------------------
/*
 * 情報表示
 * ・表示中動作停止版
 * ・文字列の長さによって自動でサイズ計算＆センタリング
 * ・基本的に１行以内用
 */
void showModalInfoDlg( const char* msg )
{
  int8_t sz = strlen(msg);
  int8_t w = sz * FONTW;

  showModalInfoDlg( 0, 0, w, FONTH, sz+1, msg );
  g_dlginfo->setPosCentering();
}

/*
 * 情報表示
 * ・表示中動作停止板
 * ・サイズ・位置指定版
 */
void showModalInfoDlg( int16_t x, int16_t y, int8_t w, int8_t h, int16_t bufsz, const char* msg )
{
  g_dlginfo = new DlgInfo( w, h, bufsz );
  g_dlginfo->setPos( x, y );
  gamemain.addAutoWindow( g_dlginfo );
  g_dlginfo->setMsg( msg );
  g_dlginfo->setBaseColor( ColorIndex::darkgray );
  g_dlginfo->setFrameColor( ColorIndex::red );
  g_dlginfo->open();
}

/*
 * 画面最下部情報表示
 * ・表示中動作停止しない
 * ・一定時間で消える
 * ・基本的に１行用
 */
void showModelessInfo( const char* msg )
{
  
}


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
void MainInit()
{
  plInit();

  enInit();

  dunInit();

  //プレイヤー配置
  DUNMAP()->enterFloor( true );

  g_dlginfo = NULL;

  g_phase = PHASE_GAME;
  g_nextphase = PHASE_NONE;
}

void MainUpdate()
{
  if ( g_dlginfo ) {
    //中断警告表示中は処理は他の処理やらない
    if ( g_dlginfo->isClosed() ) {
      delete g_dlginfo;
      g_dlginfo = NULL;
    }
  }

  //phase 切り替え
  if( g_nextphase != PHASE_NONE ) {
    changePhase();
  }
  
  if ( !g_dlginfo ) {
    switch ( g_phase ) {
      case PHASE_GAME:
        dunUpdate(); //map update

        enUpdate(); //enemy update

        plUpdate(); //player update

        UIC().update(); //ui update
        
        if ( gamemain.isTrigger( BUTTON_C ) ) {
          g_nextphase = PHASE_MENU;
        }
        break;
      case PHASE_MENU:
        if ( updateMainMenu() ) {
          g_nextphase = PHASE_GAME;
        }
        break;
      case PHASE_MENU_ITEM:
        if ( updateMenuItem() ) {
          g_nextphase = PHASE_MENU;
        }
        break;
      case PHASE_MENU_EQUIP:
        if ( updateMenuEquip() ) {
          g_nextphase = PHASE_MENU;
        }
        break;
      case PHASE_MENU_STATUS:
        if ( updateMenuStatus() ) {
          g_nextphase = PHASE_MENU;
        }
        break;
      case PHASE_MENU_OBJDROPITEM:
        if( updateMenuObjDropItem() ) {
          g_nextphase = PHASE_GAME;
        }
        break;
    }
  }
}

void MainDraw()
{
  switch ( g_phase ) {
    case PHASE_GAME:
    case PHASE_MENU:
      {
        //フレームバッファ書き換え。フレームバッファが rgb565 の時のみ対応。
        FBL().reset( fbIllumination::LVL_3 );
        //  fbl.setLight( x, y, rad );
        //  fbl.setLight( 70, 32, 20, fbIllumination::LVL_8 );

        dunDraw();

        enDraw();

        plDraw();

        FBL().apply();

        UIC().draw();
      }
      break;
    case PHASE_MENU_ITEM:
      drawMenuItem();
      break;
    case PHASE_MENU_EQUIP:
      drawMenuEquip();
      break;
    case PHASE_MENU_STATUS:
      drawMenuStatus();
      break;
    case PHASE_MENU_OBJDROPITEM:
      drawMenuObjDropItem();
      break;
  }
}

void MainFinish()
{
  dunFinish();
  enFinish();
  plFinish();

  if ( g_dlginfo ) {
    delete g_dlginfo;
    g_dlginfo = NULL;
  }
}
FLOWFUNCSET fsMain = {
  MainInit,
  MainUpdate,
  MainDraw,
  MainFinish
};

void changePhase()
{
  //現在の phase 片付け
  switch( g_phase ) {
      case PHASE_GAME:
        break;
      case PHASE_MENU:
        if( g_nextphase == PHASE_GAME ) {
          //ゲームに戻る際には破棄
          gamemain.delAutoWindow( g_win );
          delete g_win;
          g_win = NULL;
        } else {
          //item, equip, status に行く際は非表示にするだけ
          g_win->setVisible( false );
        }
        break;
      case PHASE_MENU_ITEM:
        finishMenuItem();
        g_win->setVisible( true );
        break;
      case PHASE_MENU_EQUIP:
        finishMenuEquip();
        g_win->setVisible( true );
        break;
      case PHASE_MENU_STATUS:
        g_win->setVisible( true );
        break;
      case PHASE_MENU_OBJDROPITEM:
        finishMenuObjDropItem();
        break;
  }

  g_phase = g_nextphase;
  g_nextphase = PHASE_NONE;

  //新しい phase 準備
  switch( g_phase ) {
      case PHASE_GAME:
        break;
      case PHASE_MENU:
        enterMainMenu();
        break;
      case PHASE_MENU_ITEM:
        if ( !initMenuItem() ) {
          //アイテム無くて開けない。
          showModalInfoDlg( "No Item!" ); //閉じるまで進行停止する
          g_nextphase = PHASE_MENU;
          changePhase(); //直ぐに切り替える
        }
        break;
      case PHASE_MENU_EQUIP:
        initMenuEquip();
        break;
      case PHASE_MENU_STATUS:
        break;
      case PHASE_MENU_OBJDROPITEM:
        initMenuObjDropItem();
        break;
  }
}


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
enum {
  MMITEM_ITEM,
  MMITEM_EQUIP,
  MMITEM_STATUS,
  MMITEM_RESTART,
  MMITEM_LIGHT,
  MMITEM_CLOSE,
};
void enterMainMenu()
{
  if( g_win ) return; //既に作成済みなら何もしない
  g_win = new WinSelect( 32, 4, 6 ); //gamemain.createWinSelect( 40, 4, 6 );
  gamemain.addAutoWindow( g_win );
  g_win->addItem( "Item" );
  g_win->addItem( "Equip" );
  g_win->addItem( "Status" );
  g_win->addItem( "Restart" );
  g_win->addItem( "Light" );
  g_win->addItem( "Close" );
  g_win->setPosY( 20 );
  g_win->setPosCenteringH();
  g_win->setBaseColor( ColorIndex::darkblue );
  g_win->setFrameColor( ColorIndex::yellow );
  g_win->open();
}

bool updateMainMenu()
{
  bool finish = false;
  if ( g_win->isDecide() ) {
    int8_t res = g_win->getResult();
    if ( res < 0 ) {
      //cancel
      finish = true;
    } else {
      g_win->resetDecide();
      switch ( res ) {
        //Item
        case MMITEM_ITEM:
          g_nextphase = PHASE_MENU_ITEM;
          //後で戻る様に finish はしない
          return false;
        //equip
        case MMITEM_EQUIP:
          g_nextphase = PHASE_MENU_EQUIP;
          //後で戻る様に finish はしない
          return false;
        //status
        case MMITEM_STATUS:
          g_nextphase = PHASE_MENU_STATUS;
          //後で戻る様に finish はしない
          return false;
        //DBG:restart
        case MMITEM_RESTART:
          gamemain.getFlow().setFlow( &fsMain );
          g_nextphase = PHASE_GAME; //現在の phase を終わらせ、window を破棄する為に PHASE_GAME で為に呼んでおく
          changePhase(); //直ぐに反映
          break;
        //DBG:light
        case MMITEM_LIGHT: FBL().setEnable( FBL().isEnable() ? false : true ); break;
        //DBG:cancel
        case MMITEM_CLOSE: finish = true; break;
      }
      finish = true;
    }
  } else if ( gamemain.isTrigger( BUTTON_C ) ) { //MENU による強制終了
    finish = true;
  }

  return finish;
}

//----------------------------------------------------------------
/*
   Status 画面
*/
bool updateMenuStatus()
{
  if ( gamemain.isTrigger( BUTTON_B ) ) {
    return true; //end
  }

  return false;
}

void drawMenuStatus()
{
  gb.display.setColor( ColorIndex::darkblue );
  gb.display.fillRect( 0, 0, SCRW, SCRH );
  gb.display.setColor( ColorIndex::yellow );
  gb.display.drawFastHLine( 0, 7, SCRW );

  static const int8_t TOPLINE = 8;
  static const int8_t LINEH = FONTH;
  char s[64];
  gb.display.setColor( ColorIndex::white );

  PLSTAT& ps = plGetStat();

  //Name[12], Level[99]
  sprintf( s, "%s", "Name88889ABC" );
  gb.display.setCursor( 0, TOPLINE );
  gb.display.print( s );
  sprintf( s, "L:%-2d", ps.lvl );
  gb.display.setCursor( 14 * 4, TOPLINE );
  gb.display.print( s );

  //HP[9999] / EXP[29999]
  sprintf( s, "H:%d/%d", ps.hp, ps.hpmax);
  gb.display.setCursor( 0, TOPLINE + (LINEH * 1) );
  gb.display.print( s );
  sprintf( s, "E:%d", ps.ex );
  gb.display.setCursor( 14 * 4, TOPLINE + (LINEH * 1) );
  gb.display.print( s );

  //MP[999] / GOLD[9999999]
  sprintf( s, "M:%d/%d", ps.mp, ps.mpmax );
  gb.display.setCursor( 0, TOPLINE + (LINEH * 2) );
  gb.display.print( s );
  sprintf( s, "G:%d", ps.gold );
  gb.display.setCursor( 12 * 4, TOPLINE + (LINEH * 2) );
  gb.display.print( s );

  //ATK[9999] / DEF[9999]
  sprintf( s, "ATK:%-4d DEF:%-4d", enCalcDamage( NULL ), ps.def );
  gb.display.setCursor( 0, TOPLINE + (LINEH * 3) );
  gb.display.print( s );

  //STR[999] / DEX[999]
  sprintf( s, "STR:%-3d  DEX:%-3d", ps.str, ps.dex );
  gb.display.setCursor( 0, TOPLINE + (LINEH * 4) );
  gb.display.print( s );

  //INT[999] / LUK[999]
  sprintf( s, "INT:%-3d  LUK:%-3d", ps.intl, ps.luk );
  gb.display.setCursor( 0, TOPLINE + (LINEH * 5) );
  gb.display.print( s );

  //x!x! 更に下の方に状態異常とかのアイコン並べる？
}

//----------------------------------------------------------------
/*
   Item 画面用 window
*/
class MenuItemSelect : public WinSelect
{
    static const int8_t ITEMMAX = MAX_PLITEM;
  public:
    MenuItemSelect( const char* title, uint8_t w, int8_t vline, int8_t itemlistsize, ITEM** itemlist );
    virtual ~MenuItemSelect() {
      delete[] m_itemlist;
    }

    virtual void draw();

    void rebuild(int8_t itemlistsize, ITEM** itemlist ); //アイテムを削除した際等に再構築
    inline ITEM* getItem( int8_t idx ) {
      return m_itemlist[idx];
    }
    inline int8_t getItemNum() {
      return m_itemnum;
    }
  protected:
    ITEM** m_itemlist;
    int8_t m_itemlistsize;
    char m_title[12];
};

MenuItemSelect::MenuItemSelect( const char* title, uint8_t w, int8_t vline, int8_t itemlistsize, ITEM** itemlist )
  : WinSelect( w, vline, itemlistsize )
  , m_itemlistsize( itemlistsize )
{
  m_itemlist = new ITEM*[itemlistsize];
  
  strncpy( m_title, title, 11 );
//  rebuild( itemlistsize, itemlist );
}


void MenuItemSelect::rebuild( int8_t itemlistsize, ITEM** itemlist )
{
  m_itemnum = 0;
  memset( m_itemlist, 0, sizeof(ITEM*)*itemlistsize );
  if( itemlist ) {
    for ( int8_t i = 0; i < itemlistsize; i++ ) {
      if ( itemlist[i] ) {
        m_itemlist[m_itemnum++] = itemlist[i];
      }
    }
  }

  //カーソル位置補正
  fixCursor();
}

void MenuItemSelect::draw()
{
  static const int8_t LINEH = FONTH;
  if ( !m_visible ) return;

  if ( m_stat == STAT_SELECTING || m_stat == STAT_DONESELECT ) {
    char str[48];

    //header
    gb.display.setColor( ColorIndex::black );
    gb.display.fillRect( 0, 0, SCRW, 11 );
    gb.display.setColor( ColorIndex::orange );
    gb.display.drawFastHLine( 0, 11, SCRW );
    gb.display.setColor( ColorIndex::lightgreen );
    gb.display.setCursor( 0, 4 );
    gb.display.print( m_title );


    //各アイテムは１つで２行使う
    gb.display.setColor( m_fontcol );
    int16_t x = 0, y = 12;
    for ( int8_t i = 0; i < m_vline; i++ ) {
      int8_t idx = m_top + i;
      ColorIndex bc;
      if ( ((m_cursor - m_top) == i) && (m_itemnum) ) {
//        bc = ColorIndex::blue;
        bc = ColorIndex::darkgray;
      } else {
//        bc = (idx & 1) ? ColorIndex::darkblue : ColorIndex::darkgray;
        bc = ColorIndex::black;
      }
      gb.display.setColor( bc );
      gb.display.fillRect( x, y, SCRW, LINEH * 2 );

      if ( idx < m_itemnum ) {
        ITEM* itm = m_itemlist[ idx ];

        gb.display.setColor( itGetRankColor( itm->rank ) );

#if 01
        sprintf( str, "%s%s", itGetPrefixName( itm ), itGetBaseName(itm) );
        gb.display.setCursor( x, y );
        gb.display.print( str );
        sprintf( str, "%s", itGetSuffixName( itm ) );
        gb.display.setCursor( x, y + LINEH );
        gb.display.print( str );
#else
        sprintf( str, "%s%s %s", itGetPrefixName(itm), itGetBaseName(itm), itGetSuffixName(itm) );
        gb.display.setCursor( x, y );
        gb.display.print( str );
#endif
      }

      y += LINEH * 2;
    }
  }
#if 0
  gb.display.setColor( ColorIndex::gray );
  gb.display.setCursor( 0, 6 );
  char s[64];
  sprintf( s, "c:%d t:%d v:%d %d/%d", m_cursor, m_top, m_vline, m_itemnum, m_itemmax );
  gb.display.print( s );
#endif
}

//--------------------------------------------------------------------------------------
/*
   Item 画面
*/
enum {
  MIPHASE_SELECT, //アイテム選択
  MIPHASE_ACTION, //アイテムに対する行動選択中
};
static MenuItemSelect* g_miselect = NULL;
static WinSelect* g_miaction = NULL;
static int8_t g_mitgt;
static uint8_t g_miphase;

bool initMenuItem()
{
  PLSTAT& ps = plGetStat();
  g_miselect = new MenuItemSelect( "Item", 20, 4, MAX_PLITEM, ps.items );
  g_miselect->rebuild( MAX_PLITEM, ps.items );
  if ( g_miselect->getItemNum() == 0 ) return false; //アイテムが無いと開けない
  g_miselect->open();
  g_miphase = MIPHASE_SELECT;
  return true;
}

bool updateMenuItem()
{
  switch ( g_miphase ) {
    case MIPHASE_SELECT:
      if ( g_miselect->getItemNum() == 0 ) return true; //捨てる等でアイテムが無くなればもう終わる
      g_miselect->update();
      if ( g_miselect->isDecide() ) {
        g_mitgt = g_miselect->getResult();
        if ( g_mitgt < 0 ) {
          //cancel
          return true;
        } else {
          g_miselect->resetDecide();

          g_miaction = new WinSelect( 24, 3, 3 );
          gamemain.addAutoWindow( g_miaction );
          g_miaction->addItem( "Use" );
          g_miaction->addItem( "Equip" );
          g_miaction->addItem( "Drop" );
          g_miaction->setPosY( 20 );
          g_miaction->setPosCenteringH();
          g_miaction->setBaseColor( ColorIndex::darkblue );
          g_miaction->setFrameColor( ColorIndex::yellow );
          g_miaction->open();
          g_miphase = MIPHASE_ACTION;
        }
      }
      break;
    case MIPHASE_ACTION:
      if ( g_miaction->isDecide() ) {
        int8_t res = g_miaction->getResult();
        if ( res < 0 ) {
          //cancel
        } else {
          switch ( res ) {
            case 0: //use
              break;
            case 1: //equip
              {
                ITEM* item = g_miselect->getItem( g_mitgt );
                int8_t eqpos = plCheckEquipPos( item ); //指定アイテムの装備位置

                ITEM* eqitem = plGetEquipItem( eqpos ); //装備箇所のアイテム取得
                if ( !eqitem ) {
                  //まだ装備してない
                  plEquip( item ); //装備させる
                  plDelItem( item ); //持ち物から削除
                } else {
                  //既に装備しているものと入れ替え
                  plUnequip( eqitem ); //外す
                  plEquip( item ); //装備
                  plOverwriteItem( item, eqitem ); //装備したアイテム(item)があった場所に外した装備(eqitem)上書き
                }
                g_miselect->rebuild( MAX_PLITEM, plGetStat().items );
              }
              break;
            case 2: //drop
              {
                //x!x! 捨てる前に確認入れる
                ITEM* item = g_miselect->getItem( g_mitgt );
                if ( item ) {
                  plDelItem( item ); //持ち物から削除
                  delete item;
                  g_miselect->rebuild( MAX_PLITEM, plGetStat().items );
                }
              }
              break;
          }
        }

        delete g_miaction;
        g_miphase = MIPHASE_SELECT;
      }
      break;
  }

  

  return false;
}

void drawMenuItem()
{
  g_miselect->draw();

  char str[32];
  sprintf( str, "%d/%d", plGetItemCount(), MAX_PLITEM );
  gb.display.setColor( ColorIndex::white );
  gb.display.setCursor( 13 * 4, 4 );
  gb.display.print( str );
}

void finishMenuItem()
{
  delete g_miselect;
  g_miselect = NULL;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
/*
   Equip 画面用 window
*/
class MenuEquipSelect : public WinSelect
{
    static const int8_t ITEMMAX = EQMAX;
  public:
    MenuEquipSelect( uint8_t w, int8_t vline, int8_t itemmax, ITEM** itemlist );
    virtual ~MenuEquipSelect() {}

    virtual void draw();

    void rebuild(int8_t itemmax, ITEM** itemlist ); //アイテムを削除した際等に再構築
    inline ITEM* getItem( int8_t idx ) {
      return m_itemlist[idx];
    }
    inline int8_t getItemNum() {
      return m_itemnum;
    }
  protected:
    ITEM* m_itemlist[ITEMMAX];
};

MenuEquipSelect::MenuEquipSelect( uint8_t w, int8_t vline, int8_t itemmax, ITEM** itemlist )
  : WinSelect( w, vline, itemmax )
{
  rebuild( itemmax, itemlist );
}

void MenuEquipSelect::rebuild( int8_t itemmax, ITEM** itemlist )
{
  memset( m_itemlist, 0, sizeof(ITEM*)*ITEMMAX );
  for ( m_itemnum = 0; m_itemnum < itemmax; m_itemnum++ ) {
    m_itemlist[m_itemnum] = itemlist[m_itemnum];
  }

  //カーソル位置補正
  fixCursor();
}

void MenuEquipSelect::draw()
{
  static const int8_t LINEH = FONTH;
  if ( !m_visible ) return;

  if ( m_stat == STAT_SELECTING || m_stat == STAT_DONESELECT ) {
    char str[48];

    //header
    gb.display.setColor( ColorIndex::black );
    gb.display.fillRect( 0, 0, SCRW, 11 );
    gb.display.setColor( ColorIndex::orange );
    gb.display.drawFastHLine( 0, 11, SCRW );
    gb.display.setColor( ColorIndex::lightgreen );
    gb.display.setCursor( 0, 4 );
    gb.display.print( "Equipment" );


    //各アイテムは１つで２行使う
    gb.display.setColor( m_fontcol );
    int16_t x = 0, y = 12;
    for ( int8_t i = 0; i < m_vline; i++ ) {
      int8_t idx = m_top + i;
      ColorIndex bc;
      if ( (m_cursor - m_top) == i ) {
        bc = ColorIndex::blue;
        bc = ColorIndex::darkgray;
      } else {
        bc = (idx & 1) ? ColorIndex::darkblue : ColorIndex::darkgray;
        bc = ColorIndex::black;
      }
      gb.display.setColor( bc );
      gb.display.fillRect( x, y, SCRW, LINEH * 2 );

      if ( idx < m_itemnum ) {
        ITEM* itm = m_itemlist[ idx ];

        if ( itm ) {
          //装備しているなら名前
          gb.display.setColor( itGetRankColor( itm->rank ) );

#if 01
          sprintf( str, "%s%s", itGetPrefixName( itm ), itGetBaseName(itm) );
          gb.display.setCursor( x, y );
          gb.display.print( str );
          sprintf( str, "%s", itGetSuffixName( itm ) );
          gb.display.setCursor( x, y + LINEH );
          gb.display.print( str );
#else
          sprintf( str, "%s%s %s", itGetPrefixName(itm), itGetBaseName(itm), itGetSuffixName(itm) );
          gb.display.setCursor( x, y );
          gb.display.print( str );
#endif
        } else {
          //装備していないなら部位名を表示
          gb.display.setColor( ColorIndex::gray );
          static const char* pname[] = {
            "HEAD", "WEAPON", "SHIELD", "ARMOR", "HAND", "FOOT", "AMULET", "RING",
          };

          gb.display.setCursor( x + SCRW - strlen(pname[idx]) * 4, y + 3 );
          gb.display.print( pname[idx] );
        }
      }

      y += LINEH * 2;
    }
  }
#if 0
  gb.display.setColor( ColorIndex::gray );
  gb.display.setCursor( 0, 6 );
  char s[64];
  sprintf( s, "c:%d t:%d v:%d %d/%d", m_cursor, m_top, m_vline, m_itemnum, m_itemmax );
  gb.display.print( s );
#endif
}
//-----------------------------------------------------------------------------------------------------
/*
   Equip 画面
*/
static MenuEquipSelect* g_meselect = NULL;
void initMenuEquip()
{
  PLSTAT& ps = plGetStat();
  g_meselect = new MenuEquipSelect( 20, 4, EQMAX, ps.equip );
  g_meselect->open();
  g_miphase = MIPHASE_SELECT;
}

bool updateMenuEquip()
{
  switch ( g_miphase ) {
    case MIPHASE_SELECT:
      if ( g_meselect->getItemNum() == 0 ) return true; //捨てる等でアイテムが無くなればもう終わる
      g_meselect->update();
      if ( g_meselect->isDecide() ) {
        g_mitgt = g_meselect->getResult();
        if ( g_mitgt < 0 ) {
          //cancel
          return true;
        } else {
          g_meselect->resetDecide();

          if ( g_meselect->getItem( g_mitgt ) ) { //装備されていない場所では何もしない。x!x! 装備可能なものを選べる様にする？
            g_miaction = new WinSelect( 28, 2, 2 ); //gamemain.createWinSelect( 28, 2, 2 );
            gamemain.addAutoWindow( g_miaction );
            g_miaction->addItem( "Remove" );
            g_miaction->addItem( "Use" );
            g_miaction->setPosCentering();
            g_miaction->setBaseColor( ColorIndex::darkblue );
            g_miaction->setFrameColor( ColorIndex::yellow );
            g_miaction->open();
            g_miphase = MIPHASE_ACTION;
          }
        }
      }
      break;
    case MIPHASE_ACTION:
      if ( g_miaction->isDecide() ) {
        int8_t res = g_miaction->getResult();
        if ( res < 0 ) {
          //cancel
        } else {
          g_miaction->resetDecide();
          switch ( res ) {
            case 0: //Remove
              {
                //アイテムに空きが無いと外せない
                if ( plIsItemFull() ) {
                  //空きが無い
                  showModalInfoDlg( "Item is full" ); //閉じるまで進行停止する
                  return false; //今回はここで終わる
                } else {
                  ITEM* item = plUnequip( g_mitgt ); //装備を外して
                  plAddItem( item ); //アイテムに移す
                  g_meselect->rebuild( EQMAX, plGetStat().equip );
                }
              }
              break;
            case 1: //use
              break;
          }
        }

        delete g_miaction;
        g_miphase = MIPHASE_SELECT;
      }
      break;
  }

  return false;
}

void drawMenuEquip()
{
  g_meselect->draw();
}

void finishMenuEquip()
{
  delete g_meselect;
  g_meselect = NULL;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

/*
   ObjDropItem 画面用 window
*/
class MenuObjDropItemSelect : public MenuItemSelect
{
    static const int8_t ITEMMAX = MAX_OBJECT;
  public:
    MenuObjDropItemSelect( const char* title, uint8_t w, int8_t vline, ObjBase* parent, int8_t itemlistsize, ObjBase** itemlist );
    virtual ~MenuObjDropItemSelect() {}

    void rebuild(int8_t itemlistsize, ObjBase** itemlist ); //アイテムを削除した際等に再構築
    inline ITEM* getItem( int8_t idx ) {
      return m_itemlist[idx];
    }
    inline int8_t getItemNum() {
      return m_itemnum;
    }

    void delDropObjItem( ITEM* item );
    
  protected:
    ObjBase* m_dropitemlist[ITEMMAX];
    char m_title[12];
    ObjBase* m_parent;
};

MenuObjDropItemSelect::MenuObjDropItemSelect( const char* title, uint8_t w, int8_t vline, ObjBase* parent, int8_t itemlistsize, ObjBase** itemlist )
  : MenuItemSelect( title, w, vline, itemlistsize, NULL ) //一旦空の状態で作成
  , m_parent( parent )
{
//  rebuild( itemlistsize, itemlist );
}


void MenuObjDropItemSelect::rebuild( int8_t itemlistsize, ObjBase** itemlist )
{
  //一旦 ObjDropItem を全部コピー
//  memcpy( m_dropitemlist, itemlist, sizeof(ObjBase*)*itemlistsize );
  for( int8_t i=0; i<itemlistsize; i++ ) m_dropitemlist[i] = itemlist[i];

  //m_item へアイテム取り出し
  m_itemnum = 0; //実際に有効なアイテム
  memset( m_itemlist, 0, sizeof(ITEM*)*itemlistsize );
  for ( int8_t i = 0; i < itemlistsize; i++ ) {
    if( m_dropitemlist[i] ) {
      m_itemlist[m_itemnum++] = static_cast<ObjDropItem*>(m_dropitemlist[i])->peekItem();
    }
  }

  //カーソル位置補正
  fixCursor();
}

void MenuObjDropItemSelect::delDropObjItem( ITEM* item )
{
  ObjDropItem* odi;
  for( int8_t i=0; i<m_itemlistsize; i++ ) {
    if( m_dropitemlist[i] ) {
      odi = static_cast<ObjDropItem*>(m_dropitemlist[i]);
      if( odi->peekItem() == item ) {
        if( m_parent ) { //コンテナ内の場合は中身から削除
          ObjContainer* oc = static_cast<ObjContainer*>(m_parent);
          oc->delObj( odi );
        }

        //ObjDropItem から ITEM の割当を解除
        odi->detachItem();
        
        DUNMAP()->getCurArea()->removeObj( odi ); //ObjDropItem を削除
        m_dropitemlist[i] = NULL;
        rebuild( m_itemlistsize, m_dropitemlist );
      }
    }
  }

  rebuild( m_itemlistsize, m_dropitemlist );       
}

/*
 * 複数 ObjDropItem 取得メニュー
 * title title
 * parent parent container
 * cnt ObjDropItem count
 * list ObjDropItem list
 */
void takeObjDropItemMenu( const char* title, ObjBase* parent, uint8_t cnt, ObjBase** list )
{
  //g_win 流用。MainMenu と同時に動かないので。
  g_win = new MenuObjDropItemSelect( title, 20, 4, parent, cnt, list );
  static_cast<MenuObjDropItemSelect*>( g_win )->rebuild( cnt, list );
  g_win->open();

  g_nextphase = PHASE_MENU_OBJDROPITEM;
}


void initMenuObjDropItem()
{
}

bool updateMenuObjDropItem()
{
  g_win->update();
  if( g_win->isDecide() ) {
    g_win->resetDecide();
    int8_t tgt = g_win->getResult();
    if( tgt < 0 ) {
      //終わる
      g_win->close();
      return true;
    } else {
      //入手
      if( plIsItemFull() ) { //もう持てない？
        showModalInfoDlg( "Item is full" ); //閉じるまで進行停止する
        return false;
      }

      MenuObjDropItemSelect* sel = static_cast<MenuObjDropItemSelect*>( g_win );

      //持ち物に追加
      ITEM* item = sel->getItem( tgt );
      plAddItem( item );

      //リストから削除
      sel->delDropObjItem( item );
    }
  }

  return false;
}

void drawMenuObjDropItem()
{
  g_win->draw();
}

void finishMenuObjDropItem()
{
  delete g_win;
  g_win = NULL;
}


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

