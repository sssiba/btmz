#pragma once

#include "btmz.h"

#include "gameflow.h"


#define USE_ILLUMINATION
#define USE_WINDOW


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
/*
 * 小数点表記を行いたいので、define でやって数値の場合はコンパイル時に計算出来る様にしておく
 */
//#define FIXEDPOINTBASE 64 // << 6
#define FIXEDPOINTBITS 5
#define FIXEDPOINTBASE (1<<FIXEDPOINTBITS)
#define TOFIX( v ) ((v) * FIXEDPOINTBASE)
#define TOINT( v ) ((v) / FIXEDPOINTBASE)

//1byte の整数用(char)
#define FIXMUL( a, b ) (((((int16_t)(a)) * ((int16_t)(b))) / FIXEDPOINTBASE))
#define FIXDIV( a, b ) (((int16_t)(a) * FIXEDPOINTBASE) / ((int16_t)(b)))
//2byte の整数用(int)
#define FIXLMUL( a, b ) (((((int32_t)(a)) * ((int32_t)(b))) / FIXEDPOINTBASE))
#define FIXLDIV( a, b ) (((int32_t)(a) * FIXEDPOINTBASE) / ((int32_t)(b)))


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
typedef struct {
  int8_t x;
  int8_t y;
  int8_t w;
  int8_t h;
} Rect8;


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
class WinBase;
class WinMsg;
class WinSelect;

struct GameMain {
private:
  static const uint32_t MAXPALIDX = 2;
  static const uint8_t MAX_WINDOW = 4;
  
private:
  byte btnPrvState;
  byte btnState;
  byte btnTrg;
  byte btnRep;
  byte btnRepStartFrm;
  byte btnRepIntervalFrm;
  int8_t btnRepFrm;
//  bool btnRepFlag;

  //drawCommand
  int8_t s_cx;
  int8_t s_cy;
public:
  GameFlow flow;
  static uint16_t palette[MAXPALIDX*16];
  WinBase* m_window[MAX_WINDOW];
  WinBase* m_focuswindow;
  
private:
  void readCharPGM( int num, const char*& pgm, char *d );

public:
  GameMain();
  ~GameMain();
  
  void setup();
  void cleanup();

  void checkButton();
  void update();
  void draw();
#if 0
  bool isPress( byte btn ) { return btnState & btn; }
  bool isTrigger( byte btn ) { return btnTrg & btn; }
  bool isRepeat( byte btn ) { return btnRep & btn; }
#else
  bool isPress( const Button& b ) { return gb.buttons.repeat( b, 0 ); }
  bool isTrigger( const Button& b ) { return gb.buttons.pressed( b ); }
  bool isRepeat( const Button& b ) { return gb.buttons.repeat( b, 4 ); }
#endif

  void setPalette( Gamebuino_Meta::Graphics* grp, int idx=0 ); //0 == default

  void shuffle( uint8_t* ary, uint8_t sz );

  GameFlow& getFlow() { return flow; }
  
  // drawRect() を使って bitmap 描画
  // scale は 1byte 4.4 bit の固定小数点。拡大は整数倍のみ。縮小は 1/2, 1/4, 1/8 のみ。
//  void drawRectBmp( int cx, int cy, char scale, const unsigned char *bmp, const unsigned char *mask, uint8_t color );
//  void dcDraw( const char* cmds );
//  void dcSetCursor( char x, char y );

  //-----------------------
  //Window
 /*
  WinMsg* createWinMsg( uint8_t w, uint8_t h );
  WinSelect* createWinSelect( uint8_t w, int8_t vline, int8_t itemmax );
  void destroyWindow( WinBase* win );
*/
#if defined( USE_WINDOW )

  //自動 update/draw window として登録
  bool addAutoWindow( WinBase* win );
  //自動 update/draw window から削除
  void delAutoWindow( WinBase* win );

  inline WinBase* getFocusWindow() { return m_focuswindow; }
  friend class WinBase;
private:  
  inline void setFocusWindow( WinBase* win ) { m_focuswindow = win; } //window からしか操作させない
#endif
};


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
#if defined( USE_WINDOW )
/*
 * (x,y)
 * |----------------------|
 * | marginx, marginy     |
 * |  |-----------------| |
 * |  |ABC              | |
 * |  |-----------------| |
 * |                  w,h |
 * |----------------------|
 * 
 */
class WinBase
{
public:
  enum {
    STAT_OPENING,
    STAT_OPEN,
    STAT_KEYWAIT,
    STAT_SELECTING,
    STAT_DONESELECT,
    STAT_CLOSING,
    STAT_CLOSE,
  };
public:
  WinBase( uint8_t w, uint8_t h );
  virtual ~WinBase();

  virtual void update();
  virtual void draw();

  virtual void open();
  virtual void close();
  virtual bool isOpened() { return (m_stat != STAT_CLOSING) && (m_stat != STAT_CLOSE); }
  virtual bool isClosed() { return !isOpened(); }
//  virtual void onOpened();

  inline void setBaseColor( ColorIndex c ) { m_basecol = c; }
  inline void setFrameColor( ColorIndex c ) { m_framecol = c; }
  inline void setVisible( bool f ) { m_visible = f; }

  inline void setPosX( int16_t x ) {
    m_x = x;
  }
  inline void setPosY( int16_t y ) {
    m_y = y;
  }
  inline void setPos( int16_t x, int16_t y ) {
    m_x = x;
    m_y = y;
  }

  inline void setPosCenteringH() {
    m_x = (SCRW - (m_w + m_xmargin*2))/2;
  }
  inline void setPosCenteringV() {
    m_y = (SCRH - (m_h + m_ymargin*2))/2;
  }
  inline void setPosCentering() {
    setPosCenteringH();
    setPosCenteringV();
  }

  //x!x! focus 持たない設定が必要になる予感も…
  void setFocus();
  bool isFocus();
  void unFocus();

private:
  void unlinkFocus();
  
protected:
  uint8_t m_stat;
  int16_t m_x;
  int16_t m_y;
  uint8_t m_w;
  uint8_t m_h;
  uint8_t m_xmargin;
  uint8_t m_ymargin;
  bool    m_visible;
  ColorIndex m_basecol;
  ColorIndex m_framecol;
  WinBase* m_prevfocus; //メモリもったいないかな…
  WinBase* m_nextfocus;
};



//-----------------------------------------------
/*
 * select window
 */
class WinSelect : public WinBase
{
  typedef WinBase super;
  
public:
  WinSelect( uint8_t w, int8_t vline, int8_t itemmax );
  virtual ~WinSelect();
  
  virtual void update();
  virtual void draw();
  virtual void open();

  void addItem( const char* item );

  inline void setFontColor( ColorIndex c ) { m_fontcol = c; }
  inline bool isDecide() { return m_stat == STAT_DONESELECT; }
  inline int8_t getResult() { return m_result; }
  inline void resetDecide() { m_stat = STAT_SELECTING; }

  void fixCursor();

protected:
  int8_t m_result;
  int8_t m_cursor;  //カーソル位置(item index)
  int8_t m_top;     //一番上の行の item index
  int8_t m_itemmax; //最大 item 数
  int8_t m_itemnum; //item 数
  int8_t m_vline;   //一度に表示される item 数
  int8_t m_curanm;
  ColorIndex m_fontcol;
  const char** m_item;
};

//-----------------------------------------------
/*
 * message window
 * キー入力待ち無し
 */
class WinMsg : public WinBase
{
 typedef WinBase super;
public:
  WinMsg( uint8_t w, uint8_t h, uint16_t sz = 80 );
  virtual ~WinMsg();

  virtual void update();
  virtual void draw();

  inline void setFontColor( ColorIndex c ) { m_fontcol = c; }

  void setMsg( const char* msg );
  
private:
  ColorIndex m_fontcol;
  char* m_msg;
  uint16_t m_bufsz;
};

/*
 * info dialog
 * キー入力待ち有り
 */
class DlgInfo : public WinMsg
{
  typedef WinMsg super;
public:
  DlgInfo( uint8_t w, uint8_t h, uint16_t sz = 48 );
  virtual ~DlgInfo() {}

  virtual void update();
};


#endif // USE_WINDOW

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
#if defined( USE_ILLUMINATION )
#pragma once

/*
  fb.display._buffer[] の中身を書き換える照明っぽい処理
  画面モードが rgb565 の時のみ使える。
 */

#include <Gamebuino-Meta.h>

#include "gamemain.h"

//半分のサイズのバッファで処理
//通常(80x64) -> (40x32)
//#define HALFBUF

//1/4のサイズのバッファで処理
//通常(80x64) -> (20x16)
#define QUARTERBUF

#if defined( HALFBUF ) || defined( QUARTERBUF )
#define FMT_DIRECT
#else
#define FMT_INDEXED
#endif


class fbIllumination
{
public:
  enum {
    LVL_0, //暗い
    LVL_1, 
    LVL_2,
    LVL_3,
    LVL_4,
    LVL_5,
    LVL_6,
    LVL_7,
    LVL_8,
    LVL_9,
    LVL_10,
    LVL_11,
    LVL_12,
    LVL_13,
    LVL_14,
    LVL_15,
    LVLMAX = LVL_15 //明るい
  };

#if defined( HALFBUF )
  static const int BUFW = 40;
  static const int BUFH = 32;
#elif defined( QUARTERBUF )
  static const int BUFW = 20;
  static const int BUFH = 16;
#else
  static const int BUFW = 80;
  static const int BUFH = 64;
#endif
  
public:
  fbIllumination();
  ~fbIllumination();

  void reset( uint8_t lvl = LVL_0 );
  void setLight( int16_t x, int16_t y, int16_t rad, uint8_t pow=LVLMAX );

  void apply();
  void setEnable( bool f ) { m_enable = f; }
  bool isEnable() { return m_enable; }
  void setLevelTable( const uint16_t* tbl ) { m_lvltbl = tbl; }

  /*
   * ライトを反映する領域を設定
   */
  void setClipArea( uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1 ) {
    m_clipx0 = x0;
    m_clipy0 = y0;
    m_clipx1 = x1;
    m_clipy1 = y1;
  }

private:
  uint16_t distance( int16_t dx, int16_t dy );
  inline uint8_t getLvl( uint8_t x, uint8_t y );

private:
  static const uint16_t m_defLvlTbl[LVLMAX+1];
  bool m_enable;
#if defined( FMT_INDEXED )
  uint8_t m_buf[BUFW*BUFH/2];
#else
  uint16_t m_buf[BUFW*BUFH];
#endif
  const uint16_t* m_lvltbl;
  uint8_t m_clipx0, m_clipy0, m_clipx1, m_clipy1; //適用範囲。画面サイズ以下。
  
};
#endif // USE_ILLUMINATION



//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

