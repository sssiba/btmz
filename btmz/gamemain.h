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


/*

sin
90-180 → sin( 180 - θ )
180-270 → -sin( θ - 180 )
270-360 → -sin( 360 - θ )

cos
cos(θ) → sin(θ + 90) 
 

{ //sin0 - 90 (129) [ a/90*128 -> (a*128)/90 ]
  TOFIX(0.000000f), TOFIX(0.012272f), TOFIX(0.024541f), TOFIX(0.036807f), TOFIX(0.049068f), TOFIX(0.061321f), TOFIX(0.073565f), TOFIX(0.085797f), TOFIX(0.098017f), TOFIX(0.110222f), TOFIX(0.122411f), TOFIX(0.134581f), TOFIX(0.146730f), TOFIX(0.158858f), TOFIX(0.170962f), TOFIX(0.183040f), 
  TOFIX(0.195090f), TOFIX(0.207111f), TOFIX(0.219101f), TOFIX(0.231058f), TOFIX(0.242980f), TOFIX(0.254866f), TOFIX(0.266713f), TOFIX(0.278520f), TOFIX(0.290285f), TOFIX(0.302006f), TOFIX(0.313682f), TOFIX(0.325310f), TOFIX(0.336890f), TOFIX(0.348419f), TOFIX(0.359895f), TOFIX(0.371317f), 
  TOFIX(0.382683f), TOFIX(0.393992f), TOFIX(0.405241f), TOFIX(0.416430f), TOFIX(0.427555f), TOFIX(0.438616f), TOFIX(0.449611f), TOFIX(0.460539f), TOFIX(0.471397f), TOFIX(0.482184f), TOFIX(0.492898f), TOFIX(0.503538f), TOFIX(0.514103f), TOFIX(0.524590f), TOFIX(0.534998f), TOFIX(0.545325f), 
  TOFIX(0.555570f), TOFIX(0.565732f), TOFIX(0.575808f), TOFIX(0.585798f), TOFIX(0.595699f), TOFIX(0.605511f), TOFIX(0.615232f), TOFIX(0.624860f), TOFIX(0.634393f), TOFIX(0.643832f), TOFIX(0.653173f), TOFIX(0.662416f), TOFIX(0.671559f), TOFIX(0.680601f), TOFIX(0.689541f), TOFIX(0.698376f), 
  TOFIX(0.707107f), TOFIX(0.715731f), TOFIX(0.724247f), TOFIX(0.732654f), TOFIX(0.740951f), TOFIX(0.749136f), TOFIX(0.757209f), TOFIX(0.765167f), TOFIX(0.773010f), TOFIX(0.780737f), TOFIX(0.788346f), TOFIX(0.795837f), TOFIX(0.803208f), TOFIX(0.810457f), TOFIX(0.817585f), TOFIX(0.824589f), 
  TOFIX(0.831470f), TOFIX(0.838225f), TOFIX(0.844854f), TOFIX(0.851355f), TOFIX(0.857729f), TOFIX(0.863973f), TOFIX(0.870087f), TOFIX(0.876070f), TOFIX(0.881921f), TOFIX(0.887640f), TOFIX(0.893224f), TOFIX(0.898674f), TOFIX(0.903989f), TOFIX(0.909168f), TOFIX(0.914210f), TOFIX(0.919114f), 
  TOFIX(0.923880f), TOFIX(0.928506f), TOFIX(0.932993f), TOFIX(0.937339f), TOFIX(0.941544f), TOFIX(0.945607f), TOFIX(0.949528f), TOFIX(0.953306f), TOFIX(0.956940f), TOFIX(0.960431f), TOFIX(0.963776f), TOFIX(0.966976f), TOFIX(0.970031f), TOFIX(0.972940f), TOFIX(0.975702f), TOFIX(0.978317f), 
  TOFIX(0.980785f), TOFIX(0.983105f), TOFIX(0.985278f), TOFIX(0.987301f), TOFIX(0.989177f), TOFIX(0.990903f), TOFIX(0.992480f), TOFIX(0.993907f), TOFIX(0.995185f), TOFIX(0.996313f), TOFIX(0.997290f), TOFIX(0.998118f), TOFIX(0.998795f), TOFIX(0.999322f), TOFIX(0.999699f), TOFIX(0.999925f), 
  TOFIX(1.000000f)
}

{ //sin0 - 90 (65) [ a/90*64 -> (a*64)/90 ]
  TOFIX(0.000000f), TOFIX(0.024541f), TOFIX(0.049068f), TOFIX(0.073565f), TOFIX(0.098017f), TOFIX(0.122411f), TOFIX(0.146730f), TOFIX(0.170962f), TOFIX(0.195090f), TOFIX(0.219101f), TOFIX(0.242980f), TOFIX(0.266713f), TOFIX(0.290285f), TOFIX(0.313682f), TOFIX(0.336890f), TOFIX(0.359895f),
  TOFIX(0.382683f), TOFIX(0.405241f), TOFIX(0.427555f), TOFIX(0.449611f), TOFIX(0.471397f), TOFIX(0.492898f), TOFIX(0.514103f), TOFIX(0.534998f), TOFIX(0.555570f), TOFIX(0.575808f), TOFIX(0.595699f), TOFIX(0.615232f), TOFIX(0.634393f), TOFIX(0.653173f), TOFIX(0.671559f), TOFIX(0.689541f),
  TOFIX(0.707107f), TOFIX(0.724247f), TOFIX(0.740951f), TOFIX(0.757209f), TOFIX(0.773010f), TOFIX(0.788346f), TOFIX(0.803208f), TOFIX(0.817585f), TOFIX(0.831470f), TOFIX(0.844854f), TOFIX(0.857729f), TOFIX(0.870087f), TOFIX(0.881921f), TOFIX(0.893224f), TOFIX(0.903989f), TOFIX(0.914210f),
  TOFIX(0.923880f), TOFIX(0.932993f), TOFIX(0.941544f), TOFIX(0.949528f), TOFIX(0.956940f), TOFIX(0.963776f), TOFIX(0.970031f), TOFIX(0.975702f), TOFIX(0.980785f), TOFIX(0.985278f), TOFIX(0.989177f), TOFIX(0.992480f), TOFIX(0.995185f), TOFIX(0.997290f), TOFIX(0.998795f), TOFIX(0.999699f),
  TOFIX(1.000000f)
}

{ //sin0 - 90 (91)
  TOFIX(0.000000f), TOFIX(0.017452f), TOFIX(0.034899f), TOFIX(0.052336f), TOFIX(0.069756f), TOFIX(0.087156f), TOFIX(0.104528f), TOFIX(0.121869f), TOFIX(0.139173f), TOFIX(0.156434f), TOFIX(0.173648f), TOFIX(0.190809f), TOFIX(0.207912f), TOFIX(0.224951f), TOFIX(0.241922f), TOFIX(0.258819f),
  TOFIX(0.275637f), TOFIX(0.292372f), TOFIX(0.309017f), TOFIX(0.325568f), TOFIX(0.342020f), TOFIX(0.358368f), TOFIX(0.374607f), TOFIX(0.390731f), TOFIX(0.406737f), TOFIX(0.422618f), TOFIX(0.438371f), TOFIX(0.453990f), TOFIX(0.469472f), TOFIX(0.484810f), TOFIX(0.500000f), TOFIX(0.515038f),
  TOFIX(0.529919f), TOFIX(0.544639f), TOFIX(0.559193f), TOFIX(0.573576f), TOFIX(0.587785f), TOFIX(0.601815f), TOFIX(0.615662f), TOFIX(0.629320f), TOFIX(0.642788f), TOFIX(0.656059f), TOFIX(0.669131f), TOFIX(0.681998f), TOFIX(0.694658f), TOFIX(0.707107f), TOFIX(0.719340f), TOFIX(0.731354f),
  TOFIX(0.743145f), TOFIX(0.754710f), TOFIX(0.766044f), TOFIX(0.777146f), TOFIX(0.788011f), TOFIX(0.798636f), TOFIX(0.809017f), TOFIX(0.819152f), TOFIX(0.829038f), TOFIX(0.838671f), TOFIX(0.848048f), TOFIX(0.857167f), TOFIX(0.866025f), TOFIX(0.874620f), TOFIX(0.882948f), TOFIX(0.891007f),
  TOFIX(0.898794f), TOFIX(0.906308f), TOFIX(0.913545f), TOFIX(0.920505f), TOFIX(0.927184f), TOFIX(0.933580f), TOFIX(0.939693f), TOFIX(0.945519f), TOFIX(0.951057f), TOFIX(0.956305f), TOFIX(0.961262f), TOFIX(0.965926f), TOFIX(0.970296f), TOFIX(0.974370f), TOFIX(0.978148f), TOFIX(0.981627f),
  TOFIX(0.984808f), TOFIX(0.987688f), TOFIX(0.990268f), TOFIX(0.992546f), TOFIX(0.994522f), TOFIX(0.996195f), TOFIX(0.997564f), TOFIX(0.998630f), TOFIX(0.999391f), TOFIX(0.999848f), TOFIX(1.000000f)
}
 */
#define FIXSIN( a ) gamemain.fixSin( a )
#define FIXCOS( a ) gamemain.fixCos(a )


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

  /*
   * 指定の配列の中身をシャッフルする
   */
  void shuffle( uint8_t* ary, uint8_t sz );

  /*
   * 指定の配列に、シャッフルされた 0 ～ (sz-1) までの index を入れる
   */
   void getShuffledIdx( uint8_t* ary, uint8_t sz );

  GameFlow& getFlow() { return flow; }

  /*
   * fix16 の sin を返す
   * a 角度 (fixではない通常の整数)
   */
  int16_t fixSin( int16_t a );
  /*
   * fix16 の cos を返す
   * a 角度 (fixではない通常の整数)
   */
  int16_t fixCos( int16_t a );
  
  
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
  enum {
    ATTR_VISIBLE = (1<<0),  //true:表示する
    ATTR_NOFRAME = (1<<1),  //true:フレーム無し
    ATTR_TRANSBASE = (1<<2), //true:半透過ベース
    ATTR_NOFOCUS = (1<<3),   //true:フォーカスされない
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

  inline bool isAttr( uint8_t a ) { return m_attr & a; }
  inline void setAttr( uint8_t a ) { m_attr |= a; }
  inline void clrAttr( uint8_t a ) { m_attr &= ~a; }

  inline void setBaseColor( ColorIndex c ) { m_basecol = c; }
  inline void setFrameColor( ColorIndex c ) { m_framecol = c; }
  inline void setVisible( bool f ) { f ? setAttr( ATTR_VISIBLE ) : clrAttr( ATTR_VISIBLE ); }
  inline bool isVisible() { return isAttr( ATTR_VISIBLE ); }
  inline bool isFocusable() { return !isAttr( ATTR_NOFOCUS ); } //フォーカス可能か


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

  inline void setMargin( uint8_t xm, uint8_t ym ) {
    m_xmargin = xm;
    m_ymargin = ym;
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
//  bool    m_visible;
  uint8_t m_attr;
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

/*
 * query dialog
 * ２択選択
 */
class DlgQuery : public WinMsg
{
  typedef WinMsg super;
  static const int8_t MAXSELSTR = 18;
public:
  enum : int8_t {
    RES_CANCEL = -1,
    RES_0 = 0,
    RES_1 = 1,
    RESMAX
  };
public:
  DlgQuery( uint8_t w, uint8_t h, uint16_t sz = 48 );
  virtual ~DlgQuery() {}

  virtual void draw();
  virtual void update();

  void setSel( int8_t idx, const char* msg );
  int8_t getResult() { return m_result; }
  bool isDecide() { return m_decide; }
private:
  char m_sel[RESMAX][MAXSELSTR+1];
  int8_t m_result;
  bool m_decide;
  int8_t m_curax;
};

/*
 * modeless info dialog
 * 一定時間で自動消滅
 */
class ModelessDlgInfo : public WinMsg
{
  typedef WinMsg super;

public:
  ModelessDlgInfo( uint8_t w, uint8_t h, uint16_t sz );
  virtual ~ModelessDlgInfo() {}

  virtual void update();
  inline void setDuration( int16_t dfrm ) {
    m_duration = dfrm;
  }
  inline bool isFinish() const { return !m_duration; }


private:
  int16_t m_duration; //表示期間(frame)
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

