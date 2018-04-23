#pragma once

#include "btmz.h"

#include "pic.h"

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

enum {
  OBJID_TORCH,
  OBJID_CANDLE,
  OBJID_BOX,
  OBJID_TABLE,

  MAX_OBJID
};


class ObjBase
{
public:
  enum {
    FLAG_CONTAINER   = 1 << 0, //コンテナである
    FLAG_CONTAINABLE = 1 << 1, //コンテナに格納出来る
    FLAG_CONTAINED   = 1 << 2, //コンテナに入っている
    FLAG_HOOK        = 1 << 3, //何かを接続出来る
    FLAG_HOOKABLE    = 1 << 4, //何かに接続出来る
    FLAG_HOOKED      = 1 << 5, //何かに接続されている
  };
public:
  ObjBase();
  ~ObjBase();

  virtual void init() {}
  virtual void update() {}
  virtual void draw() {}
  virtual void finish() {}

  inline uint8_t getID() { return m_id; }
  inline uint8_t getBlock() { return m_blk; }
  inline int16_t getX() { return m_x; }
  inline int16_t getY() { return m_y; }
  inline PICID getPicID() { return m_picid; }
  inline void setID( uint8_t id ) { m_id = id; }
  inline void setBlock( uint8_t blk ) { m_blk = blk; }
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

  virtual int8_t getOfstY() const { return 0; } //マップ配置時のオフセット
  virtual int8_t getOfstX() const { return 0; }

protected:
  uint8_t m_id;
  uint8_t m_blk;
  int16_t m_x;
  int16_t m_y;
  PICID m_picid;
  uint8_t m_flag;
};

class Containable : public ObjBase //入るもの
{
  typedef ObjBase super;
public:
  Containable()
  {
    setFlag( FLAG_CONTAINABLE );
  }
  virtual ~Containable() {}
};

class NotContainable : public ObjBase //入らないもの
{
  typedef ObjBase super;
public:
  NotContainable()
  {
  }
  virtual ~NotContainable() {}
};


//-----------------------------------------------
class ObjContainer : public NotContainable //コンテナはコンテナに入らない
{
  typedef NotContainable super;
public:
  static const uint8_t MAX_CONTENTS = 8; //中身最大数
  
public:
  ObjContainer();
  virtual ~ObjContainer();

  inline uint8_t getObjNum() { return m_objnum; }
  inline ObjBase** getContentsList() { return m_contents; }
  bool addObj( ObjBase* );
  bool delObj( ObjBase* );

  virtual void init();
  virtual void finish();
  
protected:
  uint8_t m_objnum; //中身の数
  ObjBase* m_contents[ MAX_CONTENTS ]; //中身Object


};

//-----------------------------------------------
class ObjHook : public NotContainable //何かをフック可能なものはコンテナに入らない
{
  typedef NotContainable super;
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
public:
  ObjTorch();
  virtual ~ObjTorch();

  virtual void init();
  virtual void update();
  virtual void draw();
private:
  uint8_t m_interval; //光マスクのちらつき間隔
  uint8_t m_fireanm;
  uint8_t m_fireanmwait;
};

//-----------------------------------------------
class ObjBox : public ObjContainer
{
  typedef ObjContainer super;
public:
  ObjBox();
  virtual ~ObjBox();
   
  virtual void init();
  virtual void update();
  virtual void draw();
  virtual void finish();

  virtual int8_t getOfstX() const { return -8; }
  virtual int8_t getOfstY() const { return 2; }
};

//-----------------------------------------------
class ObjTable : public ObjHook //テーブルの上に物が置けるので、フック可能とする
{
  typedef ObjHook super;
public:
  ObjTable();
  virtual ~ObjTable();
   
  virtual void init();
  virtual void update();
  virtual void draw();
  virtual void finish();  

  virtual void getHookedPos( int8_t idx, int16_t& x, int16_t& y );

  virtual int8_t getOfstX() const { return -8; }
  virtual int8_t getOfstY() const { return 4; }
};

//-----------------------------------------------
class ObjCandle : public Containable
{
  typedef Containable super;
public:
  ObjCandle();
  virtual ~ObjCandle();

  virtual void init();
  virtual void update();
  virtual void draw();
private:
  uint8_t m_interval; //光マスクのちらつき間隔
  uint8_t m_fireanm;
  uint8_t m_fireanmwait;
};


//-----------------------------------------------


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

