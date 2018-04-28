#include "object.h"

#include "pic.h"

#include "dungeon.h"

#include "ui.h"

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
      obj->setBlock( getBlock() ); //自分と同じブロックに設定
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

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjTorch::ObjTorch()
  : m_interval( 0 )
  , m_fireanm( 0 )
  , m_fireanmwait( 0 )
{
  setID( OBJID_TORCH );
  setPicID( PIC_OBJECT );
}

ObjTorch::~ObjTorch()
{
}

void ObjTorch::init()
{
#if 0
  DUNMAP()->getBlockTilePos( getBlock(), 1, 2, m_x, m_y );
//  m_x += 4;
//  m_y += 4;
#endif
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
  getPic( PIC_OBJECT )->setFrame( 2 );
  gb.display.drawImage( sx, sy, *getPic( PIC_OBJECT ) );
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
  setID( OBJID_CHEST );
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
  setID( OBJID_TABLE );
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
  setID( OBJID_CANDLE );
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
  getPic( PIC_OBJECT )->setFrame( 5 );
  gb.display.drawImage( sx, sy, *getPic( PIC_OBJECT ) );
  //炎
  getPic( PIC_OBJECT )->setFrame( m_fireanm ? 3 : 4 );
  gb.display.drawImage( sx, sy - 4, *getPic( getPicID() ) );

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
  setID( OBJID_UPSTAIR );
  setDrawLayer( DRAWLYR_NORMAL );
}
void ObjUpStair::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( PIC_OBJECT12x16 )->setFrame( 0 );
  gb.display.drawImage( sx, sy, *getPic( PIC_OBJECT12x16 ) );
}

ObjDownStair::ObjDownStair()
{
  setID( OBJID_DOWNSTAIR );
  setDrawLayer( DRAWLYR_NORMAL );
}

void ObjDownStair::draw()
{
  int16_t sx = DUNMAP()->toScrX( m_x );
  int16_t sy = DUNMAP()->toScrY( m_y );
  //土台
  getPic( PIC_OBJECT12x16 )->setFrame( 1 );
  gb.display.drawImage( sx, sy, *getPic( PIC_OBJECT12x16 ) );
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
ObjDropItem::ObjDropItem()
  : m_item( NULL )
{
  setDrawLayer( DRAWLYR_NEAR );
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


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------



