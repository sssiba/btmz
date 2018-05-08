#include "spatk.h"

#include "pic.h"

#include "dungeon.h"

#include "player.h"
#include "enemy.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
SpAtCtrl g_spatctrl;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
SpAtCtrl::SpAtCtrl()
{
  memset( m_spadata, 0, sizeof(m_spadata) );
}

SpAtCtrl::~SpAtCtrl()
{
}

void SpAtCtrl::update()
{
  for( int8_t i=0; i<MAX_SPATK; i++ ) {
    if( m_spadata[i].type == SPATK_NONE ) continue;

    switch( m_spadata[i].type ) {
      case SPATK_FIREBALL: updateFIREBALL( &m_spadata[i] ); break;
    }
    
  }
}

void SpAtCtrl::draw()
{
  for( int8_t i=0; i<MAX_SPATK; i++ ) {
    if( m_spadata[i].type == SPATK_NONE ) continue;
    switch( m_spadata[i].type ) {
      case SPATK_FIREBALL: drawFIREBALL( &m_spadata[i] ); break;
    }
  }
}

SpAtCtrl::SPADATA* SpAtCtrl::getEmptyData()
{
  //x!x! area 切替時、全部リセットが必要な気がする x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!x!
  
  for( int8_t i=0; i<MAX_SPATK; i++ ) {
    if( m_spadata[i].type == SPATK_NONE ) {
      return &m_spadata[i];
    }
  }
  return NULL;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
bool SpAtCtrl::createFireBall( uint8_t tgtflag, int16_t sx, int16_t sy, int16_t tx, int16_t ty, int16_t dmg )
{
  SPADATA *spad = getEmptyData();

  spad->type = SPATK_FIREBALL;
  spad->flag = tgtflag;
  spad->x = sx;
  spad->y = sy;

  if( (tx-sx) < 0 ) {
    spad->fx = TOFIX(-2.5f);
    spad->flag |= FLAG_FLIP;
  } else {
    spad->fx = TOFIX(2.5f);
  }

  if( (tx-sx) == 0 ) {
    spad->fy = 0;
  } else {
    int16_t d = TOINT(tx-sx);
    if( d < 0 ) d = -d;
    spad->fy = (ty-sy) / d;
  }
  spad->fy = FIXLMUL( spad->fy, TOFIX(2.5f) );
  
  spad->duration = 30;

  spad->value = dmg;
}

void SpAtCtrl::updateFIREBALL( SPADATA* spad )
{
  //x!x! 敵を自動追尾する？
  //x!x! 途中で敵が死ぬ場合もある
  
  spad->x += spad->fx;
  spad->y += spad->fy;
  int16_t x, y;
  x = TOINT( spad->x );
  y = TOINT( spad->y );

  Rect8 rect = { -4, -4, 8, 8 };
  //対象との当たり判定
  if( spad->flag & FLAG_TGT_ENEMY ) {
      //敵と判定
      EnemyData* ed = enCheckDfRect( x, y, rect, false );
            
      if ( ed ) {
        int16_t dmg = spad->value;
        bool dead = enDamage( ed, dmg );
        clrData( spad );
        return;
      }
  }
  if( spad->flag & FLAG_TGT_PLAYER ) {
    //プレイヤーと判定
    if( plCheckDfRect( x, y, rect, false ) ) {
      int16_t dmg = spad->value;
      plDamage( dmg );
      clrData( spad );
      return;
    }
  }

  //地形と当たり判定
  //地面・天井
  if( (y >= (((BLKTILEH-2)*TILEH+(TILEH/2))-4)) ||
      (y <= ((TILEH/2)-4))
    ) {
    //衝突
    clrData( spad );
    return;
  }
  //左右
  int16_t aw = DUNMAP()->getCurArea()->getWidth()-4;
  if( x < 4 || x >= aw ) {
    clrData(spad);
    return;
  }

  

  if( --spad->duration == 0 ) clrData( spad );
}

void SpAtCtrl::drawFIREBALL( SPADATA* spad )
{
  int8_t x, y;
  x = DUNMAP()->toScrX( TOINT(spad->x) ) - 4;
  y = DUNMAP()->toScrY( TOINT(spad->y) ) - 4;

  getPic( PIC_EFFECT8x8 )->setFrame( 2 );
  gb.display.drawImage( x, y, *getPic( PIC_EFFECT8x8 ), (spad->flag & FLAG_FLIP) ? -8 : 8, 8 );

  //光る
  FBL().setLight( x+4, y+4, 8 );
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

