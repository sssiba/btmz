/*
*/
#include "btmz.h"
#include "gamemain.h"
//#include "pic.h"
#include "title.h"

#include "player.h"

#include "object.h"

#include "dungeon.h"

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
Title::Title()
  : m_phase( PHASE_MAINMENU )
  , m_flicker( 0 )
{
  //明かり範囲設定
  FBL().setClipArea( 0, 0, 79, 63 );

  m_existsave = btmzIsSaved();

  if( m_existsave ) m_cursor = CUR_CONTINUE;
  else              m_cursor = CUR_NEW;

  m_active[CUR_NEW] = true;
  m_active[CUR_CONTINUE] = m_existsave;


  //初期化
  plInit();
  dunInit();
  enInit();
  
}

Title::~Title()
{
}

void Title::update()
{
  switch( m_phase ) {
    case PHASE_MAINMENU: updateMainMenu(); break;
    case PHASE_DECIDE: updateDecide(); break;
    case PHASE_NEW: updateNew(); break;
  }
}

void Title::draw()
{
  switch( m_phase ) {
    case PHASE_MAINMENU: drawMainMenu(); break;
    case PHASE_DECIDE: drawDecide(); break;
    case PHASE_NEW: drawNew(); break;
  }
}

//--------------------------------------------------------------------------------
void Title::updateMainMenu()
{
  FBL().reset( fbIllumination::LVL_2 );

  
  if( gamemain.isRepeat( BUTTON_DOWN ) ) {
    if( ++m_cursor >= CURMAX ) m_cursor = CURMAX-1;
  } else
  if( gamemain.isRepeat( BUTTON_UP ) ) {
    if( --m_cursor < 0 ) m_cursor = 0;
  } else
  if( gamemain.isTrigger( BUTTON_A ) && m_active[m_cursor] ) {
    m_phase = PHASE_DECIDE;
  }

  int16_t r;
  ++m_flicker;
  r = ((m_flicker/6)&1) ? 52 : 48; //overflow する辺りで間隔がおかしくなるけど、ゆらぎと言う事で気にしない。
  r -= 12;
  FBL().setLight( 32, 24, r, fbIllumination::LVL_12 );
}

void Title::drawMainMenu()
{
  //画像
  gb.display.drawImage( 0, 0, *getPic( PIC_TITLE ) ); //ほとんど暗いから一枚絵じゃなくてパーツに分けたほうが良いかも
  //炎アニメーション
  int8_t a = (m_flicker/5) % 3; //overflow する辺りで間隔がおかしくなるけど、ゆらぎと言う事で気にしない。
  getPic( PIC_TITLEFIRE )->setFrame( a );
  gb.display.drawImage( 33, 16, *getPic( PIC_TITLEFIRE ) );

  FBL().apply();

  //メニュー
  static const char* items[CURMAX] = {
    "New", "Continue",
  };


  for( int i=0; i<CURMAX; i++ ) {
    ColorIndex ci;
    if( i == m_cursor && m_active[i] ) {
      ci = ColorIndex::white;
    } else {
      ci = ColorIndex::gray;
    }
    gb.display.setColor( ci );
    int8_t len = strlen( items[i] );
    int8_t w = len*FONTW;
#if 0
    uint8_t x = (SCRW - w) / 2;
#else
    uint8_t x = (SCRW - w) - 2;
#endif
    gb.display.setCursor( x, 48+i*8 );
    gb.display.print( items[i] );

    if( i == m_cursor ) {
      gb.display.setColor( ColorIndex::orange );
      gb.display.drawFastHLine( x-2, 48+i*8+FONTH, w+4 );
    }
  }
}


//--------------------------------------------------------------------------------
void Title::updateDecide()
{
  switch( m_cursor ) {
    case CUR_NEW:
      m_phase = PHASE_NEW;
      break;
    case CUR_CONTINUE:
      if( btmzLoad() ) { //ロード
        //成功したら開始
        gamemain.getFlow().setFlow( &fsMain );
        
      } else {
        //失敗したら初められない
        m_phase = PHASE_MAINMENU;
        //再初期化
        plInit();
        dunInit();
        enInit();
      }
      break;
  }
}

void Title::drawDecide()
{
  drawMainMenu();
}

//--------------------------------------------------------------------------------
void Title::updateNew()
{
  
  //input name
  PLSTAT& ps = plGetStat();
  gb.getDefaultName( ps.name );
  gb.gui.keyboard( "Your name", ps.name );

  //map 生成
  DUNMAP()->create();

  //プレイヤー配置
  DUNMAP()->enterFloor( true );


  btmzSave();
  
  gamemain.getFlow().setFlow( &fsMain );

}

void Title::drawNew()
{
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
static Title* g_title;
void TitleInit()
{
  g_title = new Title();
}

void TitleUpdate()
{
  g_title->update();
}

void TitleDraw()
{
  g_title->draw();
}

void TitleFinish()
{
  delete g_title;
}

FLOWFUNCSET fsTitle = {
  TitleInit,
  TitleUpdate,
  TitleDraw,
  TitleFinish
};

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

