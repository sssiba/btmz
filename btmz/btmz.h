#pragma once

#include <Gamebuino-Meta.h>

#define DBG_ENABLE_SERIAL_COMM

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

#define DBG_SAVELOAD

#define SCRW 80
#define SCRH 64

#define FONTW 4
#define FONTH 6

#define MAX_AREA 24   //１マップ内の最大エリア数
#define MIN_AREA 3    //１マップ内の最低エリア数。ランダムに作るけど、これを下回る事は無いはず。
#define MAX_BLOCK 8   //１エリア内の最大 block 数
#define MAX_OBJECT 24 //１エリア内の最大 object 数

#define VER_SAVEDATA 1

#define MAX_PLITEM 16 //プレイヤーの持てるアイテム数
enum { //装備可能箇所
  EQ_HEAD,      //頭
  EQ_WEAPON,    //武器
  EQ_SHIELD,    //盾
  EQ_BODY,      //体
  EQ_HAND,      //手
  EQ_FOOT,      //靴
  EQ_AMULET,    //首飾り
  EQ_RING,      //指輪
  EQMAX
};

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
extern bool g_clrfb;
inline void enableClrFrameBuffer( bool f ) {
  g_clrfb = f;
}

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

/*
 * 情報表示
 * ・表示中動作停止版
 * ・文字列の長さによって自動でサイズ計算＆センタリング
 * ・基本的に１行以内用
 * ・showModalQueryDlg とどちらかしか同時に存在できないので同時に使わない事。
 */
extern void showModalInfoDlg( const char* msg );

/*
 * 情報表示
 * ・表示中動作停止板
 * ・サイズ・位置指定版
 * ・showModalQueryDlg とどちらかしか同時に存在できないので同時に使わない事。
 */
extern void showModalInfoDlg( int16_t x, int16_t y, int8_t w, int8_t h, int16_t bufsz, const char* msg );

/*
 * 画面最下部情報表示
 * ・表示中動作停止しない
 * ・一定時間で消える
 * ・基本的に１行用
 */
extern ModelessDlgInfo* showModelessInfo( const char* msg, uint16_t dfrm );

/*
 * ２択選択表示
 * ・表示中動作停止版
 * ・文字列の長さによって自動でサイズ計算＆センタリング
 * ・showModalInfoDlg とどちらかしか同時に存在できないので、結果を取る前に showModalInfoDlg を呼ばない事＆同時に使わない事。
 * ・ブロックするので、通常の update(), draw() は行なわれなくなる
 */
extern int8_t showModalQueryDlg( const char* title, const char* sel1, const char* sel2 );


/*
 * 複数 ObjDropItem 取得メニュー
 */
class ObjBase;
extern void takeObjDropItemMenu( const char* title, ObjBase* parent, uint8_t cnt, ObjBase** objlist );

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
enum { //save data slot
   SDS_VER,
   SDS_PLSTAT,
   SDS_PLITEM_TOP,
   SDS_PLEQUIP_TOP = SDS_PLITEM_TOP + MAX_PLITEM,
   SDS_PLFLOOR = SDS_PLEQUIP_TOP + EQMAX,
   SDSMAX
};
extern void btmzSave();

extern bool btmzLoad();

extern bool btmzIsSaved();


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

extern FLOWFUNCSET fsMain;



//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

