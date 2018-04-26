#pragma once

#include <Gamebuino-Meta.h>

//#define DBG_ENABLE_SERIAL_COMM

//#define SERIALPORT Serial
#define SERIALPORT SerialUSB //GamebuinoMeta は Serial じゃなくて SerialUSB じゃないと駄目みたい

#if defined( DBG_ENABLE_SERIAL_COMM )
  #define TRACE( ... ) SERIALPORT.println( __VA_ARGS__ ); SERIALPORT.flush()
  #define TRACEN( ... ) SERIALPORT.print( __VA_ARGS__ ); SERIALPORT.flush()
#else
  #define TRACE( ... ) (void)0
  #define TRACEN( ... ) (void)0
#endif


#define DBG_SHOW_FPS

//#define DBG_MAP //デバッグ用マップ情報表示

//#define DBG_SHOW_HITRECT //当たり判定矩形表示


#define SCRW 80
#define SCRH 64

#define MAX_AREA 24   //１マップ内の最大エリア数
#define MAX_BLOCK 8   //１エリア内の最大 block 数
#define MAX_OBJECT 24 //１エリア内の最大 object 数



//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

//フレームレート設定。60だと電池使う。
#define FRAMERATE 25

// 方向定義
enum : uint8_t {
  DIR_N,
  DIR_E,
  DIR_S,
  DIR_W,
  DIRMAX
} DIR;
#define DIRBITS 0x03

#define FBL() fbl

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------


#include "gamemain.h"
//#include "fbIllumination.h"
#include "gameflow.h"

#include "ui.h"


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

extern GameMain gamemain;
extern fbIllumination fbl;

extern const int8_t g_dirstepx[DIRMAX];
extern const int8_t g_dirstepy[DIRMAX];

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
inline int8_t getDirX( uint8_t dir ) {
  return g_dirstepx[dir];
}
inline int8_t getDirY( uint8_t dir ) {
  return g_dirstepy[dir];
}
inline void getDirV( uint8_t dir, int8_t& x, int8_t& y ) {
  x = getDirX( dir );
  y = getDirY( dir );
}
//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

/*
 * 情報表示
 * ・表示中動作停止版
 * ・文字列の長さによって自動でサイズ計算＆センタリング
 * ・基本的に１行以内用
 */
extern void showModalInfoDlg( const char* msg );

/*
 * 情報表示
 * ・表示中動作停止板
 * ・サイズ・位置指定版
 */
extern void showModalInfoDlg( int16_t x, int16_t y, int8_t w, int8_t h, int16_t bufsz, const char* msg );

/*
 * 画面最下部情報表示
 * ・表示中動作停止しない
 * ・一定時間で消える
 * ・基本的に１行用
 */
extern void showModelessInfo( const char* msg );


/*
 * 複数 ObjDropItem 取得メニュー
 */
class ObjBase;
extern void takeObjDropItemMenu( const char* title, ObjBase* parent, uint8_t cnt, ObjBase** objlist );

extern FLOWFUNCSET fsMain;



//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

