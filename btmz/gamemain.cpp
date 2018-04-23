/*
 */
#include "btmz.h"
#include "gamemain.h"


#include "utility/Misc.h"

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//color palette (RGB565)
uint16_t GameMain::palette[MAXPALIDX*16] PROGMEM = {
  //0(default)
  0xffff, 0xacd0, 0x5268, 0x0000, 0x9008, 0xca30, 0xd8e4, 0xfd42, 0xcc68, 0xfeb2, 0xf720, 0x8668, 0x044a, 0x0210, 0x4439, 0x7ddf,
  //1(color0:透明)
  (((82 & 0xf8) << 8) | //0x528a
   ((82 & 0xfc) << 3) |
   ((82 & 0xf8) >> 3)),
          0xffff, 0xacd0, 0x5268, 0x9008, 0xca30, 0xd8e4, 0xfd42, 0xcc68, 0x0000, 0xf720, 0x8668, 0x044a, 0x0210, 0x4439, 0x7ddf,
};

GameMain::GameMain()
{
#if defined( USE_WINDOW )
  m_focuswindow = NULL;
  for( int8_t i=0; i<MAX_WINDOW; i++ ) {
    m_window[i] = NULL;
  }
#endif
}

GameMain::~GameMain()
{
}


void GameMain::setup()
{
#if defined( DBG_ENABLE_SERIAL_COMM )
  //シリアル通信開始(Debug用。IDE のシリアルモニタで確認可能)
  SERIALPORT.begin( 115200 );
  SERIALPORT.println( F("serial comm start") ); //GamebuinoMeta は Serial じゃなくて SerialUSB じゃないと駄目みたい
#endif
  
  btnState = btnPrvState = btnTrg = btnRep = 0;
  btnRepStartFrm = FRAMERATE * 0.4;
  btnRepIntervalFrm = FRAMERATE * 0.2;
  btnRepFrm = 0;
//  btnRepFlag = false;

  gb.pickRandomSeed();

//  gb.display.init( 160, 128, ColorMode::index );
//  gb.display.init( 80, 64, ColorMode::index );
  gb.display.init( 80, 64, ColorMode::rgb565 );


  flow.setup();


//  flow.setFlow( &fsTitle );
flow.setFlow( &fsMain );
}

void GameMain::checkButton()
{
#if 0
  btnPrvState = btnState;
  btnState = 0;
    if (gb.buttons.pressed(BUTTON_A)) {
    }

  if( gb.buttons.pressed( UP_BUTTON ) ) btnState |= BUTTON_U;
  if( gb.buttons.pressed( RIGHT_BUTTON ) ) btnState |= BUTTON_R;
  if( gb.buttons.pressed( DOWN_BUTTON ) ) btnState |= BUTTON_D;
  if( gb.buttons.pressed( LEFT_BUTTON ) ) btnState |= BUTTON_L;
  if( gb.buttons.pressed( A_BUTTON ) ) btnState |= BUTTON_A;
  if( gb.buttons.pressed( B_BUTTON ) ) btnState |= BUTTON_B;

  byte curState = btnState ^ btnPrvState;

  btnTrg = curState & btnState;

  if( curState ) {
    btnRep = btnTrg;
    btnRepFrm = btnRepStartFrm;
  } else {
    if( --btnRepFrm <= 0 ) {
      btnRep = btnState;
      btnRepFrm = btnRepIntervalFrm;
    } else {
      btnRep = 0;
    }
  }
#endif
}

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
void GameMain::update()
{

#if defined( USE_WINDOW )
  //自動 window 更新
  for( int8_t i=0; i<MAX_WINDOW; i++ ) {
    if( m_window[i] ) m_window[i]->update();    
  }
#endif

  flow.update();
}

void GameMain::draw()
{

  //描画
  flow.draw();

#if defined( USE_WINDOW )
  //自動 window
  for( int8_t i=0; i<MAX_WINDOW; i++ ) {
    if( m_window[i] ) m_window[i]->draw();    
  }
#endif
}

void GameMain::setPalette( Gamebuino_Meta::Graphics* grp, int idx )
{
  Color* pp = (Color*)palette[ idx*16 ];

  grp->colorIndex = pp;
}

void GameMain::shuffle( uint8_t* ary, uint8_t sz )
{
  uint8_t* tmp = new uint8_t[ sz ];

  for( uint8_t i=0; i<sz; i++ ) tmp[i] = ary[i];

  uint8_t c = 0;
  while( sz ) {
    uint8_t idx = random( sz );
    ary[c++] = tmp[idx];
    tmp[idx] = tmp[--sz];
  }
  
  delete[] tmp;
}
//-------------------------------------------
//window
#if defined( USE_WINDOW )
/*
 * 自動更新 window として登録
 */
bool GameMain::addAutoWindow( WinBase* win )
{
  for( int8_t i=0; i<MAX_WINDOW; i++ ) {
    if( !m_window[i] ) {
      m_window[i] = win;
      return true;
    }
  }
  return false;
}

/*
 * 自動更新 window から抹消
 */
void GameMain::delAutoWindow( WinBase* win )
{
  for( int8_t i=0; i<MAX_WINDOW; i++ ) {
    if( m_window[i] == win ) {
      m_window[i] = NULL;
      break;
    }
  }
}
#endif


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
#if defined( USE_WINDOW )
WinBase::WinBase( uint8_t w, uint8_t h )
  : m_stat( STAT_CLOSE )
  , m_basecol( ColorIndex::black )
  , m_framecol( ColorIndex::red )
  , m_xmargin( 2 )
  , m_ymargin( 2 )
  , m_w( w )
  , m_h( h )
  , m_visible( true )
  , m_prevfocus( NULL )
  , m_nextfocus( NULL )
{
}

WinBase::~WinBase()
{
  //自分がフォーカス対象なら以前のものに切り替える
  unFocus();
  
  //登録されていなければ何もしないだけなので呼んでおく
  gamemain.delAutoWindow( this );  
}


/*
 * 自身にフォーカスがあるか返す
 */
bool WinBase::isFocus()
{
  return gamemain.getFocusWindow() == this;
}

/*
 * 自身にフォーカスする
 */
void WinBase::setFocus()
{
  WinBase* cur = gamemain.getFocusWindow();

  //x!x! ポインタで見ると削除・作成を連続した時にまずい予感
  if( cur == this ) return; //既に自分にフォーカスがあれば何もしない

  //自分が既にリンクされてるなら解除する
  unlinkFocus();

  //現在設定されている focus window を変更
  WinBase* n = NULL;
  if( cur ) {
    n = cur->m_nextfocus;
    cur->m_nextfocus = this;
  }
  m_prevfocus = cur;
  m_nextfocus = n;
  
  gamemain.setFocusWindow( this );
}

/*
 * 自身のフォーカスを無くし、他に渡す
 */
void WinBase::unFocus()
{
  if( gamemain.getFocusWindow() == this ) {
    if( m_nextfocus ) m_nextfocus->setFocus(); //フォーカスのあるものはリンクの最後にあるはずなので、こっちは無いと思うけど一応
    else
    if( m_prevfocus ) m_prevfocus->setFocus();
  }
  unlinkFocus(); //自分をフォーカスのリンクから外す
  m_nextfocus = m_prevfocus = NULL;
}

void WinBase::unlinkFocus()
{
  if( m_prevfocus ) m_prevfocus->m_nextfocus = m_nextfocus;    
  if( m_nextfocus ) m_nextfocus->m_prevfocus = m_prevfocus;
}


void WinBase::update()
{
//  if( !m_visible ) return;
}

void WinBase::draw()
{
  if( !m_visible ) return;
  if( m_stat != STAT_CLOSE ) {
    int16_t w, h;
    w = m_w + m_xmargin * 2;
    h = m_h + m_ymargin * 2;
    gb.display.setColor( m_basecol );
    gb.display.fillRect( m_x, m_y, w-1, h-1 );
    gb.display.setColor( m_framecol );
    gb.display.drawRect( m_x, m_y, w, h );
  }
}

void WinBase::open()
{
  setFocus(); //open 時フォーカスする
  m_stat = STAT_OPEN;
}

void WinBase::close()
{
  unFocus();
  m_stat = STAT_CLOSE;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
WinMsg::WinMsg( uint8_t w, uint8_t h, uint16_t sz )
  : WinBase( w, h )
  , m_fontcol( ColorIndex::white )
  , m_bufsz( sz )
{
  m_msg = new char[sz];
}

WinMsg::~WinMsg()
{
  delete[] m_msg;
}

void WinMsg::setMsg( const char* msg )
{
  strncpy( m_msg, msg, m_bufsz-1 );
  m_msg[m_bufsz-1] = '\0';
}

void WinMsg::update()
{
  super::update();

  if( !m_visible ) return;
}

void WinMsg::draw()
{
  super::draw();

  if( !m_visible ) return;
  if( m_stat != STAT_CLOSE ) {
    gb.display.setColor( m_fontcol );
    gb.display.setCursor( m_x + m_xmargin, m_y + m_ymargin );
    gb.display.print( m_msg );
  }
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
DlgInfo::DlgInfo( uint8_t w, uint8_t h, uint16_t sz )
  : WinMsg( w, h, sz )
{
}

void DlgInfo::update()
{
  if( isFocus() ) {
    if( gamemain.isTrigger( BUTTON_A ) ){
      close();
      return;
    }
  }
}


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
WinSelect::WinSelect( uint8_t w, int8_t vline, int8_t itemmax )
  : WinBase( w, 6 * vline )
  , m_fontcol( ColorIndex::white )
  , m_itemmax( itemmax )
  , m_itemnum( 0 )
  , m_vline( vline )
  , m_cursor( 0 )
  , m_result( -1 )
  , m_top( 0 )
{
  m_item = new const char*[ itemmax ];
  memset( m_item, 0, sizeof(const char*)*itemmax );
}

WinSelect::~WinSelect()
{
  delete[] m_item;
}

void WinSelect::open()
{
  super::open();
  m_stat = STAT_SELECTING;
  m_curanm = 0;
}

void WinSelect::update()
{
  super::update();

  if( !m_visible ) return;

  if( (m_stat == STAT_SELECTING) && isFocus() ) { //focus 有る時だけ操作可能
    if( gamemain.isRepeat( BUTTON_DOWN ) ) {
      ++m_cursor;
    } else
    if( gamemain.isRepeat( BUTTON_UP ) ) {
      --m_cursor;
    } else
    if( gamemain.isTrigger( BUTTON_B ) ) {
      //cancel
      m_stat = STAT_DONESELECT;
      m_result = -1;
    } else
    if( gamemain.isTrigger( BUTTON_A ) ) {
      m_result = m_cursor;
      m_stat = STAT_DONESELECT;
    }
  }
  if( m_cursor >= m_itemnum ) m_cursor = m_itemnum-1;
  if( m_cursor < 0 ) m_cursor = 0;
  m_top = m_cursor - m_vline + 2;
  if( m_top > (m_itemnum - m_vline)) m_top = m_itemnum - m_vline;
  if( m_top < 0 ) m_top = 0;
  if( (m_cursor > 0) && (m_cursor <= (m_top + 1)) ) m_top = m_cursor - 1;

#if 0
  gb.display.setColor( ColorIndex::gray );
  gb.display.setCursor( 0, 6 );
  char s[64];
  sprintf( s, "c:%d t:%d v:%d %d/%d", m_cursor, m_top, m_vline, m_itemnum, m_itemmax );
  gb.display.print( s );
#endif  
}

void WinSelect::draw()
{
  super::draw();

  if( !m_visible ) return;

  static const int8_t FONTY = 6;
  
  if( m_stat == STAT_SELECTING || m_stat == STAT_DONESELECT ) {
    gb.display.setColor( m_fontcol );
    int16_t x = m_x + m_xmargin + 4; //+cursor 分
    int16_t y = m_y + m_ymargin;
    for( int8_t i=0; i<m_vline; i++ ) {
      gb.display.setCursor( x, y + i*FONTY );
      gb.display.print( m_item[ m_top + i ] );
    }

    //x!x! focus ある時だけカーソル表示の方が良い？
    gb.display.drawChar( x - 4, y+((m_cursor-m_top)*FONTY), ((m_curanm/8) == 0) ? '>' : '-', 1 );
    if( isFocus() ) {
      //フォーカス中のみアニメーション更新
      m_curanm = (m_curanm + 1) & 0xf;
    }
  }
}

void WinSelect::addItem( const char* item )
{
  if( m_itemnum >= m_itemmax ) return;
  
  m_item[m_itemnum] = item;
  m_itemnum++;
}
#endif // USE_WINDOW

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
#if defined( USE_ILLUMINATION )
const uint16_t fbIllumination::m_defLvlTbl[] = {
  0x00, 0x18, 0x28, 0x38, 0x48, 0x60, 0x70, 0x80, 
  0xa0, 0xb0, 0xd0, 0xe0, 0xe8, 0xf0, 0xf8, 0x100,
};
fbIllumination::fbIllumination()
  : m_enable( true )
  , m_lvltbl( m_defLvlTbl )
  , m_clipx0( 0 )
  , m_clipy0( 0 )
  , m_clipx1( 79 )
  , m_clipy1( 63 )
{
  reset();
}

fbIllumination::~fbIllumination()
{
}

void fbIllumination::reset( uint8_t lvl )
{
#if defined( FMT_INDEXED )
  uint8_t c = (lvl << 4) | (lvl);
  memset( m_buf, c, sizeof(m_buf) );
#else
  for( int i=0; i<BUFW*BUFH; i++ ) m_buf[i] = m_lvltbl[lvl];
#endif

}

//http://www.flipcode.com/archives/Fast_Approximate_Distance_Functions.shtml
uint16_t fbIllumination::distance( int16_t dx, int16_t dy )
{
  uint16_t dmin, dmax, v;

  if( dx < 0 ) dx = -dx;
  if( dy < 0 ) dy = -dy;
  if( dx < dy ) {
    dmin = dx;
    dmax = dy;
  } else {
    dmin = dy;
    dmax = dx;
  }

  v = (dmax*1007) + (dmin*441);
  if( dmax < (dmin <<4) ) {
    v -= (dmax * 40);
  }
  
  return ((v + 512) >> 10);  
}



void fbIllumination::setLight( int16_t x, int16_t y, int16_t rad, uint8_t pow )
{
  if( rad <= 0 ) return;
  
  int16_t x0, y0, x1, y1;

  //必ず x0 < x1 && y0 < y1 となる
  x0 = x - rad;
  x1 = x + rad;
  y0 = y - rad;
  y1 = y + rad;
  

  //完全に描画範囲外なら何もしない
  if( (x0 < m_clipx0) && (x1 < m_clipx0) ) return;
  if( (y0 < m_clipy0) && (y1 < m_clipy0) ) return;
  if( (x0 > m_clipx1) && (x1 > m_clipx1) ) return;
  if( (y0 > m_clipy1) && (y1 > m_clipy1) ) return;
  
  
  //必ず x0 < x1 && y0 < y1 で、完全範囲外は弾かれているので、このチェックだけで足りるはず
  if( x0 < m_clipx0 ) x0 = m_clipx0;
  if( x1 > m_clipx1 ) x1 = m_clipx1;
  if( y0 < m_clipy0 ) y0 = m_clipy0;
  if( y1 > m_clipy1 ) y1 = m_clipy1;

#if defined( HALFBUF )
  x0 >>= 1;
  y0 >>= 1;
  x1 >>= 1;
  y1 >>= 1;
  x >>= 1;
  y >>= 1;
  rad >>= 1;
  if( !rad ) rad = 1;
#elif defined( QUARTERBUF )
  x0 >>= 2;
  y0 >>= 2;
  x1 >>= 2;
  y1 >>= 2;
  x >>= 2;
  y >>= 2;
  rad >>= 2;
  if( !rad ) rad = 1;
#endif  

//  float decay = pow / ((float)rad);
  uint16_t decay = (pow<<8) / rad;

  for( int ty=y0; ty<=y1; ty++ ) {
    for( int tx=x0; tx<=x1; tx++ ) {
//      int p = sqrtf( (tx-x)*(tx-x) + (ty-y)*(ty-y) ) * decay;
      int p = (distance( tx-x, ty-y ) * decay) >> 8;

      p = pow - p;
      if( p < 0 ) p = 0;

    //x!x! half とか quarter でバッファが小さければ 4bit 単位にしない方が速くて良いかも。
#if defined( FMT_INDEXED )
      uint8_t* wp = &m_buf[ (tx/2) + ((BUFW/2)*ty) ];
      uint8_t w = *wp;
      if( tx & 1 ) {
        w &= 0x0f;
        w += p;
        if( w > LVLMAX ) w = LVLMAX;
        w &= 0x0f;
        *wp = (*wp & 0xf0) | w;
      } else {
        w >>= 4;
        w += p;
        if( w > LVLMAX ) w = LVLMAX;
        w &= 0x0f;
        *wp = (w<<4) | (*wp & 0x0f);        
      }
#else
      uint16_t* wp = &m_buf[ tx + BUFW*ty ];
      *wp += m_lvltbl[ p ];
      if( *wp > 0x100 ) *wp = 0x100;
#endif
    }
  }
  
}

inline uint8_t fbIllumination::getLvl( uint8_t x, uint8_t y )
{
  //とりあえず FMT_INDEXED の時だけ対応
  uint8_t lvl;

#if defined( HALFBUF )
  x >>= 1;
  y >>= 1;
#elif defined( QUARTERBUF )
  x >>= 2;
  y >>= 2;
#endif

  lvl = m_buf[ (x/2) + ((BUFW/2)*y) ];
  
  if( x & 1 ) lvl &= 0x0f;
  else        lvl >>= 4;

  return lvl;
}

void fbIllumination::apply()
{
  if( !m_enable ) return;
  
  //フレームバッファ書き換え。フレームバッファが rgb565 の時のみ対応。

  for( int y=m_clipy0; y<=m_clipy1; y++ ) {
    uint16_t* bp = &gb.display._buffer[ y * 80 + m_clipx0 ];
    for( int x=m_clipx0; x<=m_clipx1; x++ ) {
      uint16_t c = *bp;
#if defined( FMT_INDEXED )
      uint8_t lvl = getLvl( x, y );
      int16_t cl = m_lvltbl[lvl];
#else
      int16_t cl;
      {
        uint8_t bx = x;
        uint8_t by = y;
#if defined( HALFBUF )
        bx >>= 1;
        by >>= 1;
#elif defined( QUARTERBUF )
        bx >>= 2;
        by >>= 2;
#endif
        cl = m_buf[ bx + by * BUFW ];
      }
#endif

      
#if 0
      Gamebuino_Meta::RGB888 rgb = Gamebuino_Meta::rgb565Torgb888(c);
      rgb.r = (rgb.r * cl) >> 8;
      rgb.g = (rgb.g * cl) >> 8;
      rgb.b = (rgb.b * cl) >> 8;
      c = Gamebuino_Meta::rgb888Torgb565(rgb);
#if 0
      //x!x! 色もつける？
      //x!x! 色指定用に更にメモリが必要になっちゃうか
      uint16_t r, g, b;
      r = rgb.r;
      g = rgb.g;
      b = rgb.b;
      r += 128;
      if( r > 255 ) r = 255;
      rgb.r = r;
      
      rgb.r = (rgb.r * cl) >> 8;
      rgb.g = (rgb.g * cl) >> 8;
      rgb.b = (rgb.b * cl) >> 8;
#endif

#else
      //rrrrrggggggbbbbb
      uint32_t tc;
      tc  = ((((uint32_t)(c&0xf800)) * cl) >> 8) & 0xf800;
      tc |= ((((uint32_t)(c&0x07e0)) * cl) >> 8) & 0x07e0;
      tc |= ((((uint32_t)(c&0x001f)) * cl) >> 8) & 0x001f;

     c = tc;
#endif

      *bp++ = c;
    }
  }  
}
#endif // USE_ILLUMINATION



//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

