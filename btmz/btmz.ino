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
    char s[64];
    sprintf( s, PSTR("%d"), g_fps );
    gb.display.print( s );
  }
#endif
}

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
enum {
  PHASE_GAME,
  PHASE_MENU,
  PHASE_MENU_ITEM,
  PHASE_MENU_EQUIP,
  PHASE_MENU_STATUS,
};
static uint8_t g_phase = PHASE_GAME;
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

static const int8_t FONTW = 4;
static const int8_t FONTH = 6;


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


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
void MainInit()
{
  plInit();

  enInit();

  dunInit();

#if 0
  WinMsg* w = gamemain.createWinMsg( 40, 20 );
  w->setPos( 20, 20 );
  w->setMsg( "Long Sword\nShield" );
  w->open();
#endif

  g_dlginfo = NULL;
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
  if ( !g_dlginfo ) {
    switch ( g_phase ) {
      case PHASE_GAME:
        dunUpdate();

        enUpdate();

        plUpdate();

        UIC().update();

        if ( gamemain.isTrigger( BUTTON_C ) ) {
          g_phase = PHASE_MENU;

          enterMainMenu();

        }
        break;
      case PHASE_MENU:
        {
          bool finish = updateMainMenu();
          if ( finish ) {
            g_phase = PHASE_GAME;
            gamemain.delAutoWindow( g_win );
            //          gamemain.destroyWindow( g_win );
          }
        }
        break;
      case PHASE_MENU_ITEM:
        if ( updateMenuItem() ) {
          finishMenuItem();
          g_win->setVisible( true );
          g_phase = PHASE_MENU;
        }
        break;
      case PHASE_MENU_EQUIP:
        if ( updateMenuEquip() ) {
          finishMenuEquip();
          g_win->setVisible( true );
          g_phase = PHASE_MENU;
        }
        break;
      case PHASE_MENU_STATUS:
        if ( updateMenuStatus() ) {
          g_win->setVisible( true );
          g_phase = PHASE_MENU;
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
        case 0:
          if ( initMenuItem() ) {
            g_win->setVisible( false );
            g_phase = PHASE_MENU_ITEM;
          } else {
            //アイテム無くて開けない。
            showModalInfoDlg( "No Item!" ); //閉じるまで進行停止する
          }
          //後で戻る様に finish はしない
          return false;
        //equip
        case 1:

          g_win->setVisible( false );
          initMenuEquip();
          g_phase = PHASE_MENU_EQUIP;
          //後で戻る様に finish はしない
          return false;
        //status
        case 2:
          g_win->setVisible( false );
          g_phase = PHASE_MENU_STATUS;
          //後で戻る様に finish はしない
          return false;
        //DBG:restart
        case 3: gamemain.getFlow().setFlow( &fsMain ); break;
        //DBG:light
        case 4: FBL().setEnable( FBL().isEnable() ? false : true ); break;
        //DBG:cancel
        case 5: finish = true; break;
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
    static const int8_t ITEMMAX = (MAX_PLITEM > EQMAX) ? MAX_PLITEM : EQMAX;
  public:
    MenuItemSelect( uint8_t w, int8_t vline, int8_t itemmax, ITEM** itemlist );
    virtual ~MenuItemSelect() {}

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

MenuItemSelect::MenuItemSelect( uint8_t w, int8_t vline, int8_t itemmax, ITEM** itemlist )
  : WinSelect( w, vline, itemmax )
{
  rebuild( itemmax, itemlist );
}


void MenuItemSelect::rebuild( int8_t itemmax, ITEM** itemlist )
{
  m_itemnum = 0;
  memset( m_itemlist, 0, sizeof(ITEM*)*ITEMMAX );
  for ( int8_t i = 0; i < itemmax; i++ ) {
    if ( itemlist[i] ) {
      m_itemlist[m_itemnum++] = itemlist[i];
    }
  }

  //カーソル位置補正
  int8_t scrcursor = m_cursor - m_top; //画面上のカーソル位置に変換
  //  if( m_cursor >= m_itemnum ) m_cursor = m_itemnum-1;
  //  if( m_cursor < 0 ) m_cursor = 0;
  m_top = m_cursor - m_vline + 2;
  if ( m_top > (m_itemnum - m_vline)) m_top = m_itemnum - m_vline;
  if ( m_top < 0 ) m_top = 0;

  m_cursor = m_top + scrcursor; //画面上のカーソル位置を元に実際のカーソル位置を決める
  if ( m_cursor >= m_itemnum ) m_cursor = m_itemnum - 1;
  if ( m_cursor < 0 ) m_cursor = 0;
  if ( (m_cursor > 0) && (m_cursor <= (m_top + 1)) ) m_top = m_cursor - 1;
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
    gb.display.print( "Item" );
    gb.display.setColor( ColorIndex::white );
    sprintf( str, "%d/%d", plGetItemCount(), MAX_PLITEM );
    gb.display.setCursor( 13 * 4, 4 );
    gb.display.print( str );


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
  g_miselect = new MenuItemSelect( 20, 4, MAX_PLITEM, ps.items );
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
  int8_t scrcursor = m_cursor - m_top; //画面上のカーソル位置に変換
  //  if( m_cursor >= m_itemnum ) m_cursor = m_itemnum-1;
  //  if( m_cursor < 0 ) m_cursor = 0;
  m_top = m_cursor - m_vline + 2;
  if ( m_top > (m_itemnum - m_vline)) m_top = m_itemnum - m_vline;
  if ( m_top < 0 ) m_top = 0;

  m_cursor = m_top + scrcursor; //画面上のカーソル位置を元に実際のカーソル位置を決める
  if ( m_cursor >= m_itemnum ) m_cursor = m_itemnum - 1;
  if ( m_cursor < 0 ) m_cursor = 0;
  if ( (m_cursor > 0) && (m_cursor <= (m_top + 1)) ) m_top = m_cursor - 1;
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

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

