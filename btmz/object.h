  #pragma once

#include "btmz.h"

#include "pic.h"

#include "item.h"

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
/*
 * x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!
 * ・Object は Area 毎に管理され、Area をまたいで移動する事はない。
 * ・Object の生成・破棄は、必ず Area::createObj(), Area::removeObj() を使用して行う。
 * ・基本的に、X座標は表示上の中央、Y座標は一番下を表している。
 * x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!
 */

enum : uint8_t {
  OBJID_TORCH,
  OBJID_CANDLE,
  OBJID_CHEST,
  OBJID_TABLE,
  OBJID_UPSTAIR,
  OBJID_DOWNSTAIR,
  OBJID_DROPITEM,
  OBJID_TAPESTRY,
  OBJID_STATUE,
  OBJID_FOUNTAIN,
  OBJID_SKELTON,
  OBJID_TOMB,
  OBJID_SHELF,
  OBJID_CHAIN,
  OBJID_ALL,
  MAX_OBJID = OBJID_ALL
};

enum : uint8_t { //描画順番
  DRAWLYR_WALL, //一番最初に描く。壁のもの。
  DRAWLYR_FAR, //２番めに描く。通路の奥側のもの。
  DRAWLYR_NORMAL, //３番めに描く。通路の真ん中のもの。
  DRAWLYR_NEAR, //最後に描く。通路の手前側のもの。
  MAX_DRAWLYR
};

static const uint8_t INVALID_UID = 0xff;

class ObjBase
{
  friend class Area;
public:
  enum {
    FLAG_CONTAINER   = 1 << 0, //コンテナである
    FLAG_CONTAINABLE = 1 << 1, //コンテナに格納出来る
    FLAG_CONTAINED   = 1 << 2, //コンテナに入っている
    FLAG_HOOK        = 1 << 3, //何かを接続出来る
    FLAG_HOOKABLE    = 1 << 4, //何かに接続出来る
    FLAG_HOOKED      = 1 << 5, //何かに接続されている
  };
protected:
  ObjBase();
  virtual ~ObjBase();

public:
  virtual void init() {}
  virtual void update() {}
  virtual void draw() {}
  virtual void finish() {}

  inline uint8_t getID() const { return m_id; }
  inline uint8_t getUID() { return m_uid; }
//  inline uint8_t getBlock() const { return m_blk; }
  inline int16_t getX() const { return m_x; }
  inline int16_t getY() const { return m_y; }
  inline PICID getPicID() const { return m_picid; }
//  inline void setBlock( uint8_t blk ) { m_blk = blk; }
  inline void setX( const int16_t x ) { m_x = x; }
  inline void setY( const int16_t y ) { m_y = y; }
  inline void setPos( const int16_t x, const int16_t y ) { m_x = x; m_y = y; }
  inline void setPicID( PICID id ) { m_picid = id; }
  inline bool isFlag( uint8_t flag ) const { return m_flag & flag; }
  inline bool setFlag( uint8_t flag ) { m_flag |= flag; }
  inline bool clrFlag( uint8_t flag ) { m_flag &= ~flag; }

  inline bool isContainer() const { return isFlag( FLAG_CONTAINER ); } //ものを格納出来るか
  inline bool isContainable() const { return isFlag( FLAG_CONTAINABLE ); } //コンテナに入れる事ができるか
  inline bool isContained() const { return isFlag( FLAG_CONTAINED ); } //コンテナに入っているか
  inline bool isHook() const { return isFlag( FLAG_HOOK ); } //何かを接続できるか
  inline bool isHookable() const { return isFlag( FLAG_HOOKABLE ); } //何かに接続出来るか
  inline bool isHooked() const { return isFlag( FLAG_HOOKED ); } //何かに接続中か

  inline void setDrawLayer( uint8_t lyr ) { m_layer = lyr; }
  inline uint8_t getDrawLayer() { return m_layer; }

  virtual int8_t getOfstY() const { return 0; } //マップ配置時のオフセット
  virtual int8_t getOfstX() const { return 0; }
  virtual uint8_t getAction() const { return UICtrl::BCMD_EMPTY; } //object に対して行える cmd を返す。
  virtual int8_t getActionRegionW() const { return 0; } //action に反応する幅。この幅内に player がいたら対象となる。

  virtual bool save( File& f );
  virtual bool load( File& f );
  /*
    area のobject全てをロードした後、リンクしている object のポインタの参照を解決する。
    別objectと結びつくものにだけ必要。
    area の全 object を生成後に呼ぶ事。

    areaobj ... area の管理している object の配列
   */
  virtual bool resolvePtr( ObjBase** areaobj ) {}
  static bool saveInvalidIDs( File& f ); //null の Object 用に無効な id, uid のみを書き込み
  static bool loadIDs( File& f, uint8_t& id, uint8_t& uid ); //m_id, m_uid だけ先に読む。先に読んで object 生成に利用

protected:
  inline void setID( uint8_t objid ) { m_id = objid; }
  inline void setUID( uint8_t uid ) { m_uid = uid; }
  bool resolve( ObjBase** areaobj, ObjBase** tbl, int8_t cnt );

protected:
  uint8_t m_id; //object id  (OBJID_xxx)
  uint8_t m_uid; //unique id (実際は Area::m_obj[] の index の流用)。object は area 単位で管理されるので、これで同エリア内でかぶる事は無いはず。
//  uint8_t m_blk;
  int16_t m_x;
  int16_t m_y;
  PICID m_picid;
  uint8_t m_flag;
  uint8_t m_layer;
};

class Containable : public ObjBase //入るもの
{
  typedef ObjBase super;
protected:
  Containable()
  {
    setFlag( FLAG_CONTAINABLE );
  }
  virtual ~Containable() {}
};

class NotContainable : public ObjBase //入らないもの
{
  typedef ObjBase super;
protected:
  NotContainable()
  {
  }
  virtual ~NotContainable() {}
};


//-----------------------------------------------
class ObjContainer : public NotContainable //コンテナはコンテナに入らない
{
  typedef NotContainable super;
  friend class Area;
public:
  static const uint8_t MAX_CONTENTS = 5; //中身最大数
  
protected:
  ObjContainer();
  virtual ~ObjContainer();

public:
  inline uint8_t getObjNum() { return m_objnum; }
  inline ObjBase** getContentsList() { return m_contents; }
  bool addObj( ObjBase* );
  bool delObj( ObjBase* );

  virtual void init();
  virtual void finish();

  virtual uint8_t getAction() const { return UICtrl::BCMD_LOOT; }
  
  virtual bool save( File& f );
  virtual bool load( File& f );
  virtual bool resolvePtr( ObjBase** areaobj ) {
    return resolve( areaobj, m_contents, MAX_CONTENTS );
  }
  
protected:
  uint8_t m_objnum; //中身の数
  ObjBase* m_contents[ MAX_CONTENTS ]; //中身Object
};

//-----------------------------------------------
class ObjHook : public NotContainable //何かをフック可能なものはコンテナに入らない
{ //x!x! Container を流用した方がいい？
  typedef NotContainable super;
  friend class Area;
public:
  static const uint8_t MAX_HOOKS = 2; //フック可能最大数
  
public:
  ObjHook();
  virtual ~ObjHook();

  inline uint8_t getHookNum() { return m_hooknum; }
  inline ObjBase** getHooksList() { return m_hooks; }
  bool addObj( ObjBase* );
  bool delObj( ObjBase* );

  virtual void init();
  virtual void update();
  virtual void draw();
  virtual void finish();

  virtual void getHookedPos( int8_t idx, int16_t& x, int16_t& y ) {
    x = y = 0;
  }
  
  virtual bool save( File& f );
  virtual bool load( File& f );
  virtual bool resolvePtr( ObjBase** areaobj ) {
    return resolve( areaobj, m_hooks, MAX_HOOKS );
  }
  
protected:
  uint8_t m_hooknum; //中身の数
  ObjBase* m_hooks[ MAX_HOOKS ]; //中身Object
};


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
class ObjTorch : public NotContainable
{
  typedef NotContainable super;
  friend class Area;

protected:
  ObjTorch();
  virtual ~ObjTorch();

public:
  virtual void init();
  virtual void update();
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return 0; }
private:
  uint8_t m_interval; //光マスクのちらつき間隔
  uint8_t m_fireanm;
  uint8_t m_fireanmwait;
};

//-----------------------------------------------
class ObjChest : public ObjContainer
{
  typedef ObjContainer super;
  friend class Area;
  
protected:
  ObjChest();
  virtual ~ObjChest();
   
public:
  virtual void init();
  virtual void update();
  virtual void draw();
  virtual void finish();

  virtual int8_t getOfstX() const { return -8; }
  virtual int8_t getOfstY() const { return 2; }
  virtual int8_t getActionRegionW() const { return 16; }
};

//-----------------------------------------------
class ObjTable : public ObjHook //テーブルの上に物が置けるので、フック可能とする
{
  typedef ObjHook super;
  friend class Area;
  
protected:
  ObjTable();
  virtual ~ObjTable();
   
public:
  virtual void init();
  virtual void update();
  virtual void draw();
  virtual void finish();  

  virtual void getHookedPos( int8_t idx, int16_t& x, int16_t& y );

  virtual int8_t getOfstX() const { return -8; }
  virtual int8_t getOfstY() const { return 4; }
  virtual int8_t getActionRegionW() const { return 16; }
};

//-----------------------------------------------
class ObjCandle : public Containable
{
  typedef Containable super;
  friend class Area;

protected:
  ObjCandle();
  virtual ~ObjCandle();

public:
  virtual void init();
  virtual void update();
  virtual void draw();
private:
  uint8_t m_interval; //光マスクのちらつき間隔
  uint8_t m_fireanm;
  uint8_t m_fireanmwait;
};

//-----------------------------------------------
class ObjUpStair : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjUpStair();
  virtual ~ObjUpStair() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -6; }
  virtual int8_t getOfstY() const { return 0; }

  virtual uint8_t getAction() const { return UICtrl::BCMD_UP; }
  virtual int8_t getActionRegionW() const { return 12; }
};

//-----------------------------------------------
class ObjDownStair : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjDownStair();
  virtual ~ObjDownStair() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -6; }
  virtual int8_t getOfstY() const { return 0; }

  virtual uint8_t getAction() const { return UICtrl::BCMD_DOWN; }
  virtual int8_t getActionRegionW() const { return 12; }
};

//-----------------------------------------------
/*
 * アイテムを床に落とす為のコンテナ的な物
 */
class ObjDropItem : public Containable
{
  typedef Containable super;
  friend class Area;

protected:
  explicit ObjDropItem();
  virtual ~ObjDropItem();
  
public:
  virtual void draw();
  virtual int8_t getOfstX();
  virtual int8_t getOfstY();

  virtual uint8_t getAction() const { return UICtrl::BCMD_GET; }
  virtual int8_t getActionRegionW() const;

  inline void attachItem( ITEM* item ) { m_item = item; }
  inline ITEM* peekItem() const { return m_item; }
  inline ITEM* detachItem() { //アイテムの割当を解除。解除しないとデストラクタで削除されてしまう。
    ITEM* ret = m_item;
    m_item = NULL;
    return ret;
  }

  virtual bool save( File& f );
  virtual bool load( File& f );
  
private:
  ITEM* m_item;
};

//-----------------------------------------------
/*
 * タペストリー
 */
class ObjTapestry : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjTapestry();
  virtual ~ObjTapestry() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return -12; }
};

/*
 * 彫像
 */
class ObjStatue : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjStatue();
  virtual ~ObjStatue() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return -7; }
};

/*
 * 壁の骸骨
 */
class ObjSkelton : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjSkelton();
  virtual ~ObjSkelton() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return -6; }
};

/*
 * 墓石
 */
class ObjTomb : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjTomb();
  virtual ~ObjTomb() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return -7; }
};

/*
 * 棚
 */
class ObjShelf : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjShelf();
  virtual ~ObjShelf() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return -6; } //通路に2dotはみ出し
};

/*
 * 壁の鎖
 */
class ObjChain : public NotContainable
{
  typedef NotContainable super;
  friend class Area;
  
protected:
  ObjChain();
  virtual ~ObjChain() {}

public:
  virtual void draw();
  virtual int8_t getOfstX() const { return -4; }
  virtual int8_t getOfstY() const { return -12; }
};

//-----------------------------------------------



//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

