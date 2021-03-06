#include "object.h"

#include "pic.h"

#include "dungeon.h"

#include "ui.h"

#define FS( V ) f.write( &V, sizeof(V) )
#define FL( V ) f.read( &V, sizeof(V) )

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjBase::ObjBase()
 : m_id( MAX_OBJID )
 , m_flag( 0 )
 , m_layer( DRAWLYR_FAR )
{
}

ObjBase::~ObjBase()
{
}

bool ObjBase::save( File& f )
{
  FS( m_id );
  FS( m_uid );
//  FS( m_blk );
  FS( m_x );
  FS( m_y );
  FS( m_picid );
  FS( m_flag );
  FS( m_layer );

  return true;
}

bool ObjBase::load( File& f )
{
//  FL( m_id ); x!x! loadIDs() で読む
//  FL( m_uid ); x!x! loadIDs() で読む
//  FL( m_blk );
  FL( m_x );
  FL( m_y );
  FL( m_picid );
  FL( m_flag );
  FL( m_layer );

  return true;
}

bool ObjBase::saveInvalidIDs( File& f )
{
  uint16_t d = 0xffff;
  f.write( &d, sizeof(d) );
  return true;
}

bool ObjBase::loadIDs( File& f, uint8_t& id, uint8_t& uid )
{
  //先にこれだけ読んで object 生成に利用
  f.read( &id, sizeof(id) );
  f.read( &uid, sizeof(uid) );
  return true;
}

  /*
    area のobject全てをロードした後、リンクしている object のポインタの参照を解決する。
    別objectと結びつくものにだけ必要。
    area の全 object を生成後に呼ぶ事。

    areaobj ... area の管理している object の配列
    tbl ... 変換対象となる object のリスト
    cnt ... tbl のサイズ
   */
bool ObjBase::resolve( ObjBase** areaobj, ObjBase** tbl, int8_t cnt )
{
  uint8_t uid;

  for( int8_t i=0; i<cnt; i++ ) {
    uid = (uint8_t)((uintptr_t)tbl[i]); //UID として格納されている
    if( uid == INVALID_UID ) {
      tbl[i] = NULL; //無効な UID の場所は NULL だった場所なので戻す
    } else {
      ObjBase* o = areaobj[ uid ]; //Area が管理している場所からポインタを取得
      if( !o ) return false; //自分があるべき場所が NULL だったらおかしいので失敗
      tbl[i] = o;
    }
  }

  return true;
}

//-----------------------------------------------
ObjContainer::ObjContainer()
  : m_objnum( 0 )
{
  memset( m_contents, 0, sizeof(m_contents) );
}

ObjContainer::~ObjContainer()
{
}


void ObjContainer::init()
{
}

/*
 * 入っている中身はすべて delete される
 */
void ObjContainer::finish()
{
#if 0 //x!x! Object は全て class Area で管理し、破棄もそっちでやるのでここは何もしないて良い感じ。
   //入っている中身削除
  for( int8_t i=0; i<MAX_CONTENTS; i++ ) {
    if( m_contents[i] ) {
      m_contents[i]->finish();
      delete m_contents[i];
      m_contents[i] = NULL;
    }
  }
#endif
}

bool ObjContainer::addObj( ObjBase* obj )
{
  for( int8_t i=0; i<MAX_CONTENTS; i++ ) {
    if( !m_contents[i] ) {
      m_contents[i] = obj;
      obj->setFlag( FLAG_CONTAINED );

      //格納されたコールバックみたいなの呼ぶ？

      return true;
    }
  }
  return false;
}

//コンテナから除外するだけで、obj 自体は削除しない
bool ObjContainer::delObj( ObjBase* obj )
{
  for( int8_t i=0; i<MAX_CONTENTS; i++ ) {
    if( m_contents[i] == obj ) {
      obj->clrFlag( FLAG_CONTAINED );
      m_contents[i] = NULL;
      return true;
    }
  }

  return false;
}

bool ObjContainer::save( File& f )
{
  if( !super::save( f ) ) return false;

  FS( m_objnum );
  for( uint8_t i=0; i<MAX_CONTENTS; i++ ) {
    uint8_t uid;
    if( m_contents[i] ) {
      uid = m_contents[i]->getUID(); //UID を記録しておく
    } else {
      //無い場合は無効ID
      uid = INVALID_UID;
    }
    FS( uid );
  }
  
  return true;
}

bool ObjContainer::load( File& f )
{
  if( !super::load( f ) ) return false;

  FL( m_objnum );
  for( int8_t i=0; i<MAX_CONTENTS; i++ ) {
    uint8_t uid;
    FL( uid );
    m_contents[i] = (ObjBase*)uid; //一旦 uid を格納
  }

  return true;
}

//-----------------------------------------------
ObjHook::ObjHook()
  : m_hooknum( 0 )
{
  memset( m_hooks, 0, sizeof(m_hooks) );
}

ObjHook::~ObjHook()
{
}


void ObjHook::init()
{
}

void ObjHook::update()
{
  for( int8_t i=0; i<MAX_HOOKS; i++ ) {
    if( m_hooks[i] ) {
      m_hooks[i]->update(); //x!x! 通常の update と分ける？
    }
  }
}

void ObjHook::draw()
{
  for( int8_t i=0; i<MAX_HOOKS; i++ ) {
    if( m_hooks[i] ) {
      m_hooks[i]->draw(); //x!x! 通常の draw と分ける？
    }
  }
}

/*
 * 接続されているものは全て delete される
 */
void ObjHook::finish()
{
#if 0 //x!x! Object は全て class Area で管理し、破棄もそっちでやるのでここは何もしないて良い感じ。
  //入っている中身削除
  for( int8_t i=0; i<MAX_HOOKS; i++ ) {
    if( m_hooks[i] ) {
      m_hooks[i]->finish();
      delete m_hooks[i];
      m_hooks[i] = NULL;
    }
  }
#endif
}

bool ObjHook::addObj( ObjBase* obj )
{
  for( int8_t i=0; i<MAX_HOOKS; i++ ) {
    if( !m_hooks[i] ) {
      m_hooks[i] = obj;
      m_hooknum++;

      //接続されたコールバックみたいなの呼ぶ？
//      obj->setBlock( getBlock() ); //自分と同じブロックに設定
      obj->init(); //x!x! ここで初期化。new した直後に接続したらこれが必要。だけど既に初期化済みだったらまずい？初期化済みフラグつけて最初期化されないようにする？
      obj->setFlag( FLAG_HOOKED );

      //全部の座標を修正。
      for( int8_t j=0; j<MAX_HOOKS; j++ ) {
        if( m_hooks[j] ) {
          int16_t x, y;
          getHookedPos( j, x, y );
          m_hooks[j]->setPos( x, y );
        }
      }

      return true;
    }
  }
  return false;
}

//フックから除外するだけで、obj 自体は削除しない
bool ObjHook::delObj( ObjBase* obj )
{
  for( int8_t i=0; i<MAX_HOOKS; i++ ) {
    if( m_hooks[i] == obj ) {
      obj->clrFlag( FLAG_HOOKED );
      m_hooknum--;
      m_hooks[i] = NULL;
      return true;
    }
  }

  return false;
}
bool ObjHook::save( File& f )
{
  if( !super::save( f ) ) return false;

  FS( m_hooknum );
  for( uint8_t i=0; i<MAX_HOOKS; i++ ) {
    uint8_t uid;
    if( m_hooks[i] ) {
      uid = m_hooks[i]->getUID(); //UID を記録しておく
    } else {
      //無い場合は無効ID
      uid = INVALID_UID;
    }
    FS( uid );
  }
  
  return true;
}

bool ObjHook::load( File& f )
{
  if( !super::load( f ) ) return false;

  FL( m_hooknum );
  for( int8_t i=0; i<MAX_HOOKS; i++ ) {
    uint8_t uid;
    FL( uid );
    m_hooks[i] = (ObjBase*)uid; //一旦 uid を格納
  }

  return true;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjTorch::ObjTorch()
  : m_interval( 0 )
  , m_fireanm( 0 )
  , m_fireanmwait( 0 )
{
  setDrawLayer( DRAWLYR_WALL );
  setPicID( PIC_OBJECT );
}

ObjTorch::~ObjTorch()
{
}

void ObjTorch::init()
{
}

void ObjTorch::update()
{
  if( ++m_fireanmwait == 4 ) {
    m_fireanm = m_fireanm ^ 1;
    m_fireanmwait = 0;
  }
}

void ObjTorch::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( getPicID() )->setFrame( 2 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
  //炎
  getPic( PIC_OBJECT )->setFrame( m_fireanm ? 1 : 0 );
  gb.display.drawImage( sx, sy - 5, *getPic( getPicID() ) );

  //ライト
  //x!x! QUARTERBUF の場合、座標は４の倍数の場所にないと、スクロール時にずれる
#if !defined( QUARTERBUF )
//  FBL().setLight( sx+4, sy + 2, (m_interval & 1) ? 40 : 36 );
  FBL().setLight( sx+4, sy + 2, (m_interval & 1) ? 48 : 44 );
#else
  FBL().setLight( sx+4, sy + 2, (m_interval & 1) ? 48 : 44 );
#endif

  m_interval ^= 1;

}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjChest::ObjChest()
{
  setPicID( PIC_OBJECT16x8 );
}

ObjChest::~ObjChest()
{
}

void ObjChest::init()
{
//  DUNMAP()->getBlockTilePos( getBlock(), 2, 3, m_x, m_y );  
//  m_y += 2;
}

void ObjChest::update()
{
  
}

void ObjChest::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( getPicID() )->setFrame( 0 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

void ObjChest::finish()
{
  super::finish();
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjTable::ObjTable()
{
  setPicID( PIC_OBJECT16x8 );
}

ObjTable::~ObjTable()
{
}

void ObjTable::init()
{
//  DUNMAP()->getBlockTilePos( getBlock(), 2, 3, m_x, m_y );  
//  m_y += 4;
}

void ObjTable::update()
{
  super::update();
  
}

void ObjTable::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( getPicID() )->setFrame( 2 ); //足
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
  getPic( getPicID() )->setFrame( 1 ); //机の上
  gb.display.drawImage( sx, sy-8, *getPic( getPicID() ) );

  //フックしているものを描画しないと駄目
  super::draw();
}

void ObjTable::finish()
{
  super::finish();  
}

void ObjTable::getHookedPos( int8_t idx, int16_t& x, int16_t& y )
{
  //x!x! ２個まで置け、個数によって場所が違う。 サイズによって置ける数制限する？
  if( getHookNum() == 1 ) {
    x = m_x + 4;
    y = m_y - 9;
  } else {
    x = m_x + 4 - 4 + 8 * idx;
    y = m_y - 9;
  }
}


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjCandle::ObjCandle()
  : m_interval( 0 )
  , m_fireanm( 0 )
  , m_fireanmwait( 0 )
{
  setPicID( PIC_OBJECT );
}

ObjCandle::~ObjCandle()
{
}

void ObjCandle::init()
{
//  DUNMAP()->getBlockTilePos( getBlock(), 2, 3, m_x, m_y );
//  m_x += 4;
//  m_y += 4;
}

void ObjCandle::update()
{
  if( ++m_fireanmwait == 4 ) {
    m_fireanm = m_fireanm ^ 1;
    m_fireanmwait = 0;
  }
}

void ObjCandle::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( getPicID() )->setFrame( 5 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
  //炎
  getPic( PIC_OBJECT )->setFrame( m_fireanm ? 3 : 4 );
  gb.display.drawImage( sx, sy - 4, *getPic( PIC_OBJECT ) );

  //ライト
#if !defined( QUARTERBUF )
//  FBL().setLight( sx+4, sy + 2, (m_interval & 1) ? 40 : 36 );
  FBL().setLight( sx+4, sy - 0, (m_interval & 1) ? 20 : 16 );
#else
  FBL().setLight( sx+4, sy - 0, (m_interval & 1) ? 20 : 16 );
#endif

  m_interval ^= 1;

}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjUpStair::ObjUpStair()
{
  setDrawLayer( DRAWLYR_NORMAL );
  setPicID( PIC_OBJECT12x16 );
}
void ObjUpStair::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( getPicID() )->setFrame( 0 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//------------------------------------------------
ObjDownStair::ObjDownStair()
{
  setDrawLayer( DRAWLYR_NORMAL );
  setPicID( PIC_OBJECT12x16 );
}

void ObjDownStair::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( getPicID() )->setFrame( 1 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjDropItem::ObjDropItem()
  : m_item( NULL )
{
  setDrawLayer( DRAWLYR_NEAR );
  setPicID(PICINVALID); //x!x! アイテム依存なので定義出来ない
}

ObjDropItem::~ObjDropItem()
{
  if( m_item ) { //アイテムが接続されたままの場合はここで破棄する
    delete m_item;
  }
}

void ObjDropItem::draw()
{
  if( !isContained() ) { //何かに収納されている場合は描かない
    ItemPic* ip = itGetItemBase( m_item->base )->pic;

    int16_t sx = DUNMAP()->toScrX( m_x );
    int16_t sy = DUNMAP()->toScrY( m_y );
  
    ip->pic->setFrame( ip->frm );
    gb.display.drawImage( sx, sy, *ip->pic );  
  }
}

int8_t ObjDropItem::getOfstX()
{
  ItemPic* ip = itGetItemBase( m_item->base )->pic;
  return -ip->w/2;
} 

int8_t ObjDropItem::getOfstY()
{
  ItemPic* ip = itGetItemBase( m_item->base )->pic;
  return -ip->h/2;
}

int8_t ObjDropItem::getActionRegionW() const
{
  ItemPic* ip = itGetItemBase( m_item->base )->pic;
  return ip->w;
}

bool ObjDropItem::save( File& f )
{
  if( !super::save( f ) ) return false;

  itSave( f, m_item );
  
  return true;
}

bool ObjDropItem::load( File& f )
{
  if( !super::load( f ) ) return false;

  m_item = itLoad( f );
  
  return true;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjTapestry::ObjTapestry()
{
  setDrawLayer( DRAWLYR_WALL );
  setPicID( PIC_OBJECT8x16 );
}
void ObjTapestry::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  getPic( getPicID() )->setFrame( 0 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjStatue::ObjStatue()
{
  setPicID( PIC_OBJECT8x16 );
}
void ObjStatue::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  getPic( getPicID() )->setFrame( 1 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjSkelton::ObjSkelton()
{
  setPicID( PIC_OBJECT8x16 );
}
void ObjSkelton::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  getPic( getPicID() )->setFrame( 2 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjTomb::ObjTomb()
{
  setPicID( PIC_OBJECT8x16 );
}
void ObjTomb::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  getPic( getPicID() )->setFrame( 3 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjShelf::ObjShelf()
{
  setPicID( PIC_OBJECT8x16 );
}
void ObjShelf::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  getPic( getPicID() )->setFrame( 4 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjChain::ObjChain()
{
  setPicID( PIC_OBJECT8x16 );
}
void ObjChain::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  getPic( getPicID() )->setFrame( 5 );
  gb.display.drawImage( sx, sy, *getPic( getPicID() ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------



