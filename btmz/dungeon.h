#pragma once

#include <Gamebuino-Meta.h>

#include "gamemain.h"

#include "object.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
#define BLKTILEW 6 //１ブロックの横幅(タイル単位)
#define BLKTILEH 6
#define TILEW 8    //１タイルの横幅
#define TILEH 8

//仮想マップのセルサイズ
#define TMAPW 10
#define TMAPH 10

//エリア切替時の入口方向による位置オフセット
#define ENTEROFSTH (TILEW * (BLKTILEW/2-1) + (TILEW/2)-2)
#define ENTEROFSTV 4


enum BlockDir : uint8_t {
  BDIR_FAR,   //奥
  BDIR_RIGHT, //右
  BDIR_NEAR,  //手前
  BDIR_LEFT,  //左
  BDIRMAX
};

enum BlockDirInfo : uint8_t {
  BDINFO_EMPTY,
  BDINFO_CORRIDOR, //通路
  BDINFO_WALL, //壁
  BDINFO_DOOR, //ドア
};

typedef union {
  struct { //通路の場合
    uint8_t toArea;
    uint8_t toBlock;
  } corridor;
  struct { //ドアの場合
    uint8_t toArea;
    uint8_t toBlock;
    uint8_t prm;
  } door;
} BDIDATA;

enum ObjType {
  OBJ_EMPTY,
  OBJ_UPSTAIR,
  OBJ_DOWNSTAIR,
};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
/*
 * マップ構築時に使用される仮想マップ
 * これで作成した仮想マップを元に、横スクロール用マップを生成
 */
class CellMaker
{
public:
    enum { //entrance
      E_WALL, E_CORRIDOR, E_NOTHING, E_DOOR,
    };
    enum : int8_t { //cellobject
      O_EMPTY, O_UPSTAIR, O_DOWNSTAIR, O_FOUNTAIN, O_ALTAR,
    };
    struct CELL {
      uint8_t id;
      uint8_t dist; //開始地点からの距離。実際のマップでは、何番目のブロックかになる。
      uint8_t entrance;      //2bit 単位(0x03 のシフトで判定するので、0bWWSSEENN)で、NESW の接続部分情報を表す(E_WALL, E_CORRIDOR, ...)
      uint8_t toArea[DIRMAX]; //各方向の接続先エリア
      uint8_t toBlock[DIRMAX]; //各方向の接続先ブロック(エリア内の場所)
      uint8_t mapobject;  //階段等のマップに紐づく object
      uint8_t attr; //エリアの属性を元に各cell毎に細かく設定される
    };
    struct AREABASE {
      int8_t sx, sy;
      uint8_t dir; //エリアを伸ばした方向
      uint8_t len;
      uint8_t attr; //属性
      uint8_t rtype; //部屋の場合の種類
      uint8_t droplvlcorrection; //ドロップレベル補正
      uint8_t enemynum;
      inline void fwdPos( int8_t& x, int8_t& y ) { x += getDirX(dir); y += getDirY(dir); }
    };

    //生成するフロアのタイプ
    //タイプ毎に特徴をつける？
    enum : uint8_t {
      FTYPE_NORMAL,
      FTYPE_CEMETERY, //墓場系(cemetery, altar, ...)
      FTYPE_DUNGEON, //ダンジョン系
      FTYPE_WIZARD, //魔法使いの研究所
      FTYPE_ARMY, //軍事施設(barrack, throne, prison, hall, ...)
      FTYPE_ELDER, //古代遺跡(ruin, altar, fountain, ...)
      MAXFTYPE
    };
    //属性
    enum : uint8_t {
      AATTR_DARK = (1<<0), //暗闇(明かりが（ほぼ）配置されない？)
      AATTR_ENEMY = (1<<1), //敵(敵がいる）
      AATTR_ITEMDROP = (1<<2), //アイテムが落ちている
      AATTR_TREASURE = (1<<3), //宝(部屋にのみ配置？)
      AATTR_TRAP = (1<<4),
      AATTR_ROOM = (1<<5), //部屋(部屋である) -> 部屋じゃない場所 == 通路
    };
    //部屋の種類（フロアのタイプ、フロアによってどれがあるか決まる）
    enum : uint8_t {
      RTYPE_BARRACKS, //兵舎（敵が一杯？強い敵がいる？）
      RTYPE_PRIVATEROOM, //個室（テーブルとか家具がある？）
      RTYPE_PRISON, //牢獄（壁が牢屋になってる？）
      RTYPE_THRONE, //玉座の間（ボス的なのがいる？）
      RTYPE_FOUNTAIN, //泉の間（色々な効果の泉がある？）
      RTYPE_ALTAR, //祭壇の間（色々な効果のある祭壇がある）
      RTYPE_CEMETERY, //墓地（骸骨とか幽霊とか？）
      RTYPE_LABORATORY, //実験室（テーブルと実験器具？）
      RTYPE_RUIN, //廃墟(強的と強いアイテムがある？ぼろぼろの壁？）
      RTYPE_TREASURE, //宝物庫(必ず１個以上宝箱がある？）
      RTYPE_HALL, //広間（大きいエリアのみ？なんかある？）
      MAXRTYPE
    };
    //部屋の情報
    typedef struct {
      uint8_t minsize; //最低blockサイズ
      uint8_t itemrate; //アイテムが落ちてる割合(0-100)
      uint8_t chestrate; //宝箱がある割合(0-100)
      uint8_t droplvlcorrection; //ドロップレベル補正
      uint8_t enemyrate; //敵出現率
      uint8_t minenemy; //敵最小数
      uint8_t maxenemy; //敵最大数
    } ROOMDATA;

private:
  typedef bool (*InitRoomFunc)( CellMaker*, AREABASE* );
  static bool initRoomBarracks( CellMaker* cm, AREABASE* ab );
  static bool initRoomPrivateRoom( CellMaker* cm, AREABASE* ab );
  static bool initRoomPrison( CellMaker* cm, AREABASE* ab );
  static bool initRoomThrone( CellMaker* cm, AREABASE* ab );
  static bool initRoomFountain( CellMaker* cm, AREABASE* ab );
  static bool initRoomAltar( CellMaker* cm, AREABASE* ab );
  static bool initRoomCemetery( CellMaker* cm, AREABASE* ab );
  static bool initRoomLaboratory( CellMaker* cm, AREABASE* ab );
  static bool initRoomRuin( CellMaker* cm, AREABASE* ab );
  static bool initRoomTreasure( CellMaker* cm, AREABASE* ab );
  static bool initRoomHall( CellMaker* cm, AREABASE* ab );
    
public:
  CellMaker();
  ~CellMaker();

  void make( uint8_t mapfloor );

  inline uint8_t getAreaCount() { return m_areacnt; }
  inline AREABASE* getAreaBase( int8_t idx ) { return &m_areabase[idx]; }
  inline CELL* getCell( int8_t x, int8_t y ) { return &m_cell[ x + y * TMAPW ]; }
  CELL* getCellFromAreaDist( uint8_t area, uint8_t dist ); //指定エリアの開始位置から、指定された距離にある CELL を返す

  bool randomObject( int8_t cellobj );

  void makeRoom();
  void setupCell();

#if defined( DBG_MAP )
  void DBGdumpMap();
#endif

private:
  void initConnect( uint8_t id );
  bool findNextStart( uint8_t curid, int8_t& x, int8_t& y, uint8_t& dir, int8_t& nx, int8_t& ny );

private:
  CELL m_cell[ TMAPW * TMAPH ];
  AREABASE m_areabase[ MAX_AREA ];
  uint8_t m_areacnt;
  static const ROOMDATA m_roomdata[ MAXRTYPE ];
  uint8_t m_tgtfloor;
};


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
class Area;
#if defined( DBG_MAP )
class Map;
#endif
/*
 * １ブロックは幅４タイル 
 */
class Block
{
public:
#if defined( DBG_MAP )
  friend class Map;
  friend class Area;
#endif
  Block() {}
  Block( Area* area, CellMaker* cm, CellMaker::AREABASE* abase, CellMaker::CELL* cell );
  ~Block();


  void draw( const uint8_t* bg, int16_t x, int16_t y );
  void setInfo( uint8_t dir, BlockDirInfo info );
  inline BlockDirInfo getInfo( uint8_t dir ) {
    return m_dirinfo[dir];
  }
  BDIDATA* getBDIData( uint8_t dir ) { return &m_bdidata[dir]; }
  inline uint8_t getDist() { return m_dist; }

  void makeBG( uint8_t* out );

  bool isWall( uint8_t dir ); //指定の方向が壁か調べる

  void setObjWall( ObjBase* obj ); //壁に置く（基本中央。通路やドアならその左右のどっちか）
  void setObjGround( ObjBase* obj ); //床（奥の方）に置く
  void setObjCenter( ObjBase* obj ); //床（中央）に置く
  void setObjCeiling( ObjBase* obj ); //天井（中央）に置く

  //指定のエリア：ブロックと接続されている方向を返す
  //BDIRMAX が返った場合は見つからなかった。
  BlockDir findConnectDir( int8_t area, int8_t blk );


  bool save( File& f );
  bool load( File& f );
   
private:
  void writeBGparts( const uint8_t* parts, uint8_t* out );
  
private:
  BlockDirInfo m_dirinfo[BDIRMAX]; //左右奥手前の４方向の状況。壁とかドアとか何も無いとか。
  BDIDATA m_bdidata[BDIRMAX];      //各方向の状況用パラメータ
  uint8_t m_dist;
};

//--------------------------------------------------------------------------
/*
 * エリアはブロックの集合
 */
class Area
{
public:
#if defined( DBG_MAP )
  friend class Map;
#endif
  Area();
  ~Area();

  void setup( CellMaker* cm, uint8_t area );
  void draw();
  void update();

  Block* getBlock( uint8_t blk ) { return m_blk[blk]; }
  inline uint8_t getBlockCnt() { return m_blkcnt; }

  BlockDir getEnterPos( uint8_t blk, int8_t prvarea, int8_t prvblk, int16_t& x, int16_t& y );

  void makeBG( uint8_t* out );

  int16_t getWidth(); //横幅(dot)を返す

#if defined( DBG_MAP )
  //for Debug
  uint8_t DBGgetDir() { return m_DBGdir; }
  uint8_t DBGgetSX() { return m_DBGsx; }
  uint8_t DBGgetSY() { return m_DBGsy; }
#endif

  /*
   * Object の生成
   * x!x! Object は全てこれを使用して生成する事。勝手に作ると、Area 削除時に自動で削除されない事になるかも。
   */
  ObjBase* createObj( uint8_t blk, uint8_t objid );
  bool entryObj( uint8_t blk, ObjBase* obj );
  ObjBase* getObj( uint8_t idx );
  void removeObj( ObjBase* obj );
  void removeObj( uint8_t idx );
  void setObjPosWall( ObjBase* obj );
  void setObjPosGround( ObjBase* obj );
  
  inline void getBlockTilePos( uint8_t blk, uint8_t bx, uint8_t by, int16_t& x, int16_t& y )
  {
    x = (blk * BLKTILEW + bx) * TILEW;
    y = by * TILEH;
  }

  /*
   * コンテナの中身を入れる
   */
   void setupContainer( ObjContainer* objc, uint8_t mapfloor, uint8_t droplvl );

   bool save( File& f );
   bool load( File& f );

private:
  ObjBase* createObjInstance( uint8_t objid );

protected:
  uint8_t m_blkcnt;
  Block** m_blk;
  ObjBase* m_obj[MAX_OBJECT];

#if defined( DBG_MAP )
public:
  uint8_t m_DBGdir; //DBG: 仮想マップ上でのエリアの伸びてる方向
  uint8_t m_DBGsx, m_DBGsy; //DBG: 仮想マップ上でのエリアの開始位置
#endif
};

//--------------------------------------------------------------------------
/*
 * マップはエリアの集合
 */
class Map
{
public:
  typedef struct _OBJFINDER {
    int8_t area;
    int8_t objidx;
    uint8_t objid;

    _OBJFINDER( uint8_t findobjid )
    {
      area = -1;
      objidx = -1;
      objid = findobjid;
    }
  } OBJFINDER;
public:
  Map();
  ~Map();

  void draw();
  void create( uint8_t mapfloor );
  void update();

  inline int16_t getHomeX() { return m_homex; }
  inline int16_t getHomeY() { return m_homey; }
  inline int16_t setHomeX( int16_t hx ) { m_homex = hx; }
  inline void getHome( int16_t& x, int16_t& y ) { x = m_homex; y = m_homey; } //ホームポジションを取得
  inline Area* getCurArea() { return m_area[m_curareaidx]; } //現在の area を取得
  inline uint8_t getCurAreaIdx() { return m_curareaidx; } //現在の area の index を取得
  inline uint8_t* getAreaBG() { return m_areaBG; } //現在の area の BG map を取得
  Block* getBlock( int16_t x, int16_t y );

  /*
   * 指定の object id を持つ object を探す
   * 最初に見つかったものを返す。複数あるものを順次見つける場合は、同じ OBJFINDER を使って呼び続ける。
   * objid に OBJID_ALL を指定すると全ての object を拾う
   */
  ObjBase* findObject( OBJFINDER& of );

  /*
   * 別フロアからマップに入る
   */
  void enterFloor( bool descend );

  /*
   * 指定エリアの指定ブロックに入る
   * area 新たに入るエリア
   * blk 新たに入るブロック
   * prvarea 以前にいたエリア
   * prvblk 以前にいたブロック
   * return 0:奥側から進入 1:右から進入 2:手前から進入 3:左から進入
   */
  BlockDir enter( int8_t area, int8_t blk, int8_t prvarea, int8_t prvblk );

  //マップ上の X 座標を画面座標に変換
  inline int16_t toScrX( int16_t x )
  {
    return x - m_homex;
  }

  //マップ上の Y 座標を画面座標に変換
  inline int16_t toScrY( int16_t y )
  {
    return y - m_homey;
  }

  //指定位置(マップ座標)のBGを取得
  uint8_t getMapBG( int16_t x, int16_t y );

  //指定ブロックの指定タイルの、現在のエリアの座標上の位置を取得
  inline void getBlockTilePos( uint8_t blk, uint8_t bx, uint8_t by, int16_t& x, int16_t& y )
  {
    getCurArea()->getBlockTilePos( blk, bx, by, x, y );
  }

  //save する
  bool save();

  //load する
  bool load();

  inline uint8_t getMapFloor() { return m_mapfloor; }


#if defined( DBG_MAP )
  void DBGout();
#endif

protected:
  uint8_t m_mapfloor; //現在保持しているマップのフロア
  uint8_t m_areacnt;
  Area** m_area;
  int16_t m_homex, m_homey;
  uint8_t m_curareaidx;

  /*
   * 各ブロックが１ライン毎に並ぶのではなく、
   * 00: 001122
   * 06: 001122
   * 0c: 001122
   * ↓の様にブロック丸ごと単位に並ぶ
   * 00: 000000
   * 06: 111111
   * 0c: 222222
   */
  uint8_t m_areaBG[BLKTILEW*BLKTILEH*MAX_BLOCK]; //x!x! 引数で渡さなくても良い様にグローバル変数にする？
};


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

extern void dunInit();
extern void dunUpdate();
extern void dunDraw();
extern void dunFinish();

extern Map* g_map;

#define DUNMAP() (g_map)

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


