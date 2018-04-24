#include "btmz.h"

#include "dungeon.h"

#include "pic.h"

#include "player.h"

#include "enemy.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Map* g_map = NULL;

void dunCreate();

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void dunCreate()
{
  if ( g_map ) {
    delete g_map;
  }
  g_map = new Map;
  g_map->create();

}


void dunInit()
{
  dunCreate();
}

void dunUpdate()
{
  g_map->update();
}


void dunDraw()
{
  g_map->draw();

  //  for( int i=0; i<4*8; i++ ) {
  //    dungeonbg.setFrame( 1 );
  //    gb.display.drawImage( i*8, 16, dungeonbg );
  //  }


}

void dunFinish()
{
  if ( g_map ) {
    delete g_map;
    g_map = NULL;
  }
}



//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
/*
   BG Parts
   x, y, w, h, data ...,
*/
static const uint8_t farWall[] PROGMEM = { //奥の壁
  0, 0, //x, y
  4, 5, //w, h
  0, 0, 0, 0,
  1, 1, 1, 1,
  1, 1, 1, 1,
  1, 1, 1, 1,
  0, 0, 0, 0,
};
static const uint8_t farCorridor[] PROGMEM = { //奥の通路
  0, 0, //x, y
  4, 5, //w, h
  0, 0, 0, 0,
  1, 2, 5, 1,
  1, 3, 6, 1,
  1, 4, 7, 1,
  0, 0, 0, 0,
};
static const uint8_t farDoor[] PROGMEM = { //奥のドア x!x! ドア部分だけ後から書くようにした方がいいかも
  0, 0, //x, y
  4, 5, //w, h
  0, 0, 0, 0,
  1, 1, 1, 1,
  1, 18, 20, 1,
  1, 19, 21, 1,
  0, 0, 0, 0,
};


static const uint8_t nearWall[] PROGMEM = { //手前の壁
  0, 5, //x, y
  4, 1, //w, h
  1, 1, 1, 1,
};
static const uint8_t nearCorridor[] PROGMEM = { //手前の通路
  0, 5, //x, y
  4, 1, //w, h
  4, 0, 0, 7,
};
static const uint8_t nearDoor[] PROGMEM = { //手前のドア
  0, 5, //x, y
  4, 1, //w, h
  22, 23, 24, 25,
};

static const uint8_t leftWall[] PROGMEM = { //左の壁
  0, 0,
  1, 5,
  8, 9, 10, 11, 12,
};
static const uint8_t leftDoor[] PROGMEM = { //左のドア
  0, 0,
  1, 5,
  8, 9, 26, 27, 28,
};

static const uint8_t rightWall[] PROGMEM = { //右の壁
  3, 0,
  1, 5,
  13, 14, 15, 16, 17,
};

static const uint8_t rightDoor[] PROGMEM = { //右のドア
  3, 0,
  1, 5,
  13, 14, 29, 30, 31,
};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Block::Block( Area* area, CellMaker* cm, CellMaker::AREABASE* abase, CellMaker::CELL* cell )
{
  uint8_t dir = abase->dir; //ブロックの方向(エリアが伸びていく方向)
  uint8_t rdir = (dir + 2) & 0x3; //ブロックの方向の逆方向

  m_dist = cell->dist;

  //仮想マップのNSEWの方向を BDIR の方向に変換するテーブル
  static const uint8_t dirtobdir[DIRMAX][BDIRMAX] PROGMEM = {
    //N E S W
    { BDIR_RIGHT, BDIR_NEAR, BDIR_LEFT, BDIR_FAR  }, //dir が DIR_N の時のNESWの変換。N が RIGHT 方向
    { BDIR_FAR, BDIR_RIGHT, BDIR_NEAR, BDIR_LEFT }, //dir が DIR_E の時
    { BDIR_LEFT, BDIR_FAR, BDIR_RIGHT, BDIR_NEAR }, //dir が DIR_S の時
    { BDIR_NEAR, BDIR_LEFT, BDIR_FAR, BDIR_RIGHT }, //dir が DIR_W の時
  };

  //entrance : WWSSEENN
  for ( int8_t i = 0; i < 4; i++ ) {
    uint8_t entrance = (cell->entrance >> (i * 2)) & 0x03;
    uint8_t bdir = dirtobdir[dir][i];
    BlockDirInfo* bdi = &m_dirinfo[ bdir ];
    BDIDATA* bdid = &m_bdidata[ bdir ];

    switch ( entrance ) {
      case CellMaker::E_CORRIDOR:
        {
          *bdi = BDINFO_CORRIDOR;
          //通路のリンク先エリア
          bdid->corridor.toArea = cell->toArea[i];
          bdid->corridor.toBlock = cell->toBlock[i];

#if defined( DBG_MAP )
          {
            static const char dn[] = { 'N', 'E', 'S', 'W' };
            static const char bn[] = { 'F', 'R', 'N', 'L' };
            char s[80];
            sprintf( s, "[%02d_%02d]corridor: dir:%c  cell[%c:%d_%d] -> blk[%c:%d_%d]", cell->id - 1, m_dist, dn[dir], dn[i], cell->toArea[i], cell->toBlock[i], bn[bdir], bdid->corridor.toArea, bdid->corridor.toBlock );
            TRACE( s );
          }
#endif
        }
        break;
      case CellMaker::E_DOOR:
        {
          *bdi = BDINFO_DOOR;
          //通路のリンク先エリア
          bdid->door.toArea = cell->toArea[i];
          bdid->door.toBlock = cell->toBlock[i];
          bdid->door.prm = 0; //x!x! 鍵かかってるとか設定

#if defined( DBG_MAP )
          {
            static const char dn[] = { 'N', 'E', 'S', 'W' };
            static const char bn[] = { 'F', 'R', 'N', 'L' };
            char s[80];
            sprintf( s, "[%02d_%02d]door: dir:%c  prm:%08x cell[%c:%d_%d] -> blk[%c:%d_%d]", cell->id - 1, m_dist, dn[dir], bdid->door.prm, dn[i], cell->toArea[i], cell->toBlock[i], bn[bdir], bdid->door.toArea, bdid->door.toBlock );
            TRACE( s );
          }
#endif
        }
        break;
      case CellMaker::E_WALL:
        {
          *bdi = BDINFO_WALL;
        }
        break;
      default:
        {
          *bdi = BDINFO_EMPTY;
        }
        break;
    }
  }

  //map object
  switch( cell->mapobject ) {
    case CellMaker::O_UPSTAIR:
      {
        ObjUpStair* obj = static_cast<ObjUpStair*>( area->createObj( m_dist, OBJID_UPSTAIR ) );
        setObjCeiling( obj );
      }
      break;
    case CellMaker::O_DOWNSTAIR:
      {
        ObjDownStair* obj = static_cast<ObjDownStair*>( area->createObj( m_dist, OBJID_DOWNSTAIR ) );
        setObjGround( obj );
      }
      break;
  }
  
}

Block::~Block()
{
}


void Block::setInfo( uint8_t dir, BlockDirInfo info )
{
  m_dirinfo[dir] = info;
}

void Block::writeBGparts( const uint8_t* parts, uint8_t* out )
{
  if ( !parts ) return;

  uint8_t x, y, w, h;
  x = *parts++;
  y = *parts++;
  w = x + *parts++;
  h = y + *parts++;
  for ( int i = y; i < h; i++ ) {
    for ( int j = x; j < w; j++ ) {
      out[ j + i * BLKTILEW ] = *parts++;
    }
  }
}

void Block::draw( const uint8_t* bg, int16_t x, int16_t y )
{
  int16_t tx, ty, dx, dy;

  for ( int i = 0; i < BLKTILEH; i++ ) {
    for ( int j = 0; j < BLKTILEW; j++ ) {
      getPic( PIC_DUNGEONBG )->setFrame( bg[ j + i * BLKTILEW ] );
      gb.display.drawImage( x + TILEW * j, y + TILEH * i, *getPic( PIC_DUNGEONBG ) );
    }
  }
}

bool Block::isWall( uint8_t dir )
{
  return m_dirinfo[ dir ] == BDINFO_WALL;
}

void Block::makeBG( uint8_t* out )
{
  const uint8_t* bg;

  //奥
  switch ( m_dirinfo[BDIR_FAR] ) {
    case BDINFO_WALL: bg = farWall; break;
    case BDINFO_CORRIDOR: bg = farCorridor; break;
    case BDINFO_DOOR: bg = farDoor; break;
  }
  writeBGparts( bg, out );

  //左
  switch ( m_dirinfo[BDIR_LEFT] ) {
    case BDINFO_WALL: bg = leftWall; break;
    case BDINFO_CORRIDOR: bg = NULL; break;
    case BDINFO_DOOR: bg = leftDoor; break;
  }
  writeBGparts( bg, out );

  //右
  switch ( m_dirinfo[BDIR_RIGHT] ) {
    case BDINFO_WALL: bg = rightWall; break;
    case BDINFO_CORRIDOR: bg = NULL; break;
    case BDINFO_DOOR: bg = rightDoor; break;
  }
  writeBGparts( bg, out );

  //手前
  switch ( m_dirinfo[BDIR_NEAR] ) {
    case BDINFO_WALL: bg = nearWall; break;
    case BDINFO_CORRIDOR: bg = nearCorridor; break;
    case BDINFO_DOOR: bg = nearDoor; break;
  }
  writeBGparts( bg, out );

}

/*
    指定の object を壁に配置する
*/
void Block::setObjWall( ObjBase* obj )
{
  int16_t x, y;
  uint8_t bx, by;
  bool center = false;
  by = 2;
  if ( isWall( BDIR_FAR ) ) { //奥側が壁なら中央に配置
    bx = 1;
    center = true;
  } else {
    //通路・ドアなら左右のどっちかに配置。両方もあり？
    bx = (random(100) & 1) ? 0 : 3;
  }

  x = ( m_dist * BLKTILEW + bx) * TILEW + (center ? 4 : 0);
  y = by * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );
}

/*
   指定の object を床(奥側)に配置する
*/
void Block::setObjGround( ObjBase* obj )
{
  int16_t x, y;
  x = ( m_dist * BLKTILEW + 2) * TILEW;
  y = 3 * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );
}

void Block::setObjCenter( ObjBase* obj )
{
  int16_t x, y;
  x = ( m_dist * BLKTILEW + 2) * TILEW;
  y = 4 * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );
}

void Block::setObjCeiling( ObjBase* obj )
{
  int16_t x, y;
  x = ( m_dist * BLKTILEW + 2) * TILEW;
  y = 0 * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );

}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Area::Area()
  : m_blkcnt( 0 )
  , m_blk( NULL )
{
  memset( m_obj, 0, sizeof(m_obj) );
}

Area::~Area()
{
  if ( m_blk ) {
    for ( int i = 0; i < m_blkcnt; i++ ) delete m_blk[i];
    delete[] m_blk;
  }

  for ( int i = 0; i < MAX_OBJECT; i++ ) {
    if ( m_obj[i] ) {
      delete m_obj[i];
    }
  }
}

void Area::setup( CellMaker* cm, uint8_t id )
{
  CellMaker::AREABASE *abp = cm->getAreaBase( id );
  m_blkcnt = abp->len;
  m_blk = new Block*[m_blkcnt];

  int8_t x, y;
  x = abp->sx;
  y = abp->sy;

  for ( int i = 0; i < m_blkcnt; i++ ) {
    CellMaker::CELL* cell = cm->getCell( x, y );
    m_blk[i] = new Block( this, cm, abp, cell );

    abp->fwdPos( x, y );

#if 01
    //object 配置test
    if ( random(100) < 40 ) {
      if ( random(100) < 75 ) {
        ObjBase* o = createObj( i, OBJID_TORCH );
        m_blk[i]->setObjWall( o ); //壁に配置
      } else {
        ObjBase* o = createObj( i, OBJID_CANDLE );
        m_blk[i]->setObjGround( o );  //床に配置
      }
    }
#if 01 //container
    if ( random(100) < 20 ) {
      if ( random(100) < 30 ) {
        ObjBase* o = createObj( i, OBJID_BOX );
        m_blk[i]->setObjGround( o );
      } else {
        ObjTable* tbl = static_cast<ObjTable*>( createObj( i, OBJID_TABLE ) );
        m_blk[i]->setObjGround( tbl );
        //直接作成して机に置く
        tbl->addObj( new ObjCandle() ); //ろうそくを机におく
      }
    }
#endif
#if 01 //item
    if( random(100) < 30 ) {
      ObjDropItem* o = static_cast<ObjDropItem*>( createObj( i, OBJID_DROPITEM ) );
      if( o ) {
        ITEM* item = itGenerateFloor( plGetFloor() );
        o->setItem( item );
        m_blk[i]->setObjCenter( o ); //通路上に置く
      }

      //同じ場所に２個おいて実験
       o = static_cast<ObjDropItem*>( createObj( i, OBJID_DROPITEM ) );
      if( o ) {
        ITEM* item = itGenerateFloor( plGetFloor() );
        o->setItem( item );
        m_blk[i]->setObjCenter( o ); //通路上に置く
      }
    
    
    
    }
#endif
#endif

  }

#if 01 //敵配置 test
    if( m_blkcnt > 1 ) {
      enCreate( ENTYPE_SLIME, 0, id, random(m_blkcnt-1) + 1 );
    }
#endif

#if defined( DBG_MAP )
  //for DEBUG
  m_DBGsx = abp->sx;
  m_DBGsy = abp->sy;
  m_DBGdir = abp->dir;
#endif
}

void Area::draw()
{
  int16_t hx, hy;
  hx = DUNMAP()->getHomeX();
  hy = DUNMAP()->getHomeY();

  //背景
  uint8_t* bg = DUNMAP()->getAreaBG();

  for ( int i = 0; i < m_blkcnt; i++ ) {
    int16_t bx, by;

    bx = -hx + i * BLKTILEW * TILEW;
    by = -hy;

    m_blk[i]->draw( bg, bx, by );

    bg += BLKTILEW * BLKTILEH;
  }

#if 01
  //x!x! object 間の描画のプライオリティを付けないとまずい。階段とたいまつの位置関係等がおかしくなる。 x!x!
  //object
  for ( int i = 0; i < MAX_OBJECT; i++ ) {
    if ( m_obj[i] ) {
      m_obj[i]->draw();
    }
  }
#endif
}

void Area::update()
{
#if 01
  //object
  for ( int i = 0; i < MAX_OBJECT; i++ ) {
    if ( m_obj[i] ) {
      m_obj[i]->update();
    }
  }
#endif
}

void Area::getEnterPos( uint8_t blk, int16_t& x, int16_t& y )
{
  x = blk * BLKTILEW * TILEW + (BLKTILEW * TILEW / 2);
  y = (BLKTILEH - 2) * TILEH + (TILEH / 2);
}

/*
   背景BGを生成して格納する。
   保持する全ブロック分を一度に作成。
*/
void Area::makeBG( uint8_t* out )
{
  for ( int i = 0; i < m_blkcnt; i++ ) {
    m_blk[i]->makeBG( out );
    out += BLKTILEW * BLKTILEH;
  }
}

int16_t Area::getWidth()
{
  return m_blkcnt * BLKTILEW * TILEW;
}

ObjBase* Area::createObj( uint8_t blk, uint8_t objid )
{
  ObjBase* obj = NULL;
  switch ( objid ) {
    case OBJID_TORCH: obj = new ObjTorch(); break;
    case OBJID_CANDLE: obj = new ObjCandle(); break;
    case OBJID_BOX: obj = new ObjBox(); break;
    case OBJID_TABLE: obj = new ObjTable(); break;
    case OBJID_UPSTAIR: obj = new ObjUpStair(); break;
    case OBJID_DOWNSTAIR: obj = new ObjDownStair(); break;
    case OBJID_DROPITEM: obj = new ObjDropItem(); break;
  }

  if ( obj ) {
    if ( !entryObj( blk, obj ) ) {
      //登録出来なかった
      delete obj;
      obj = NULL;
    } else {
      //登録出来た
    }
  }

  return obj;
}

bool Area::entryObj( uint8_t blk, ObjBase* obj )
{
  uint8_t i;
  for ( i = 0; i < MAX_OBJECT; i++ ) {
    if ( !m_obj[i] ) break;
  }
  if ( i == MAX_OBJECT ) return false; //もう登録出来ない

  m_obj[i] = obj;
  obj->setBlock( blk );
  obj->init();

  return true;
}

void Area::setObjPosWall( ObjBase* obj )
{
}

void Area::setObjPosGround( ObjBase* obj )
{
}

ObjBase* Area::getObj( uint8_t idx )
{
  return m_obj[idx];
}

void Area::removeObj( ObjBase* obj )
{
  obj->finish();
  delete obj;

  for ( uint8_t i = 0; i < MAX_OBJECT; i++ ) {
    if ( m_obj[i] == obj ) {
      uint8_t j;
      for ( j = i; j < MAX_OBJECT - 1; j++ ) {
        m_obj[j] = m_obj[j + 1];
      }
      m_obj[j] = NULL;
      return;
    }
  }
}

void Area::removeObj( uint8_t idx )
{
  removeObj( m_obj[idx] );
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Map::Map()
  : m_areacnt( 0 )
  , m_area( NULL )
  , m_curareaidx( 0 )
  , m_homey( -16 ) //画面上１６ドットは空欄
{
}

Map::~Map()
{
  if ( m_area ) {
    for ( int i = 0; i < m_areacnt; i++ ) delete m_area[i];
    delete[] m_area;
  }
}

void Map::create()
{
  //仮想マップ構築
  CellMaker* cm = new CellMaker();
  cm->make();

  //仮想マップの情報を元にエリア作成
  m_areacnt = cm->getAreaCount();
  m_area = new Area*[m_areacnt];
  for ( int i = 0; i < m_areacnt; i++ ) {
    m_area[i] = new Area;
    m_area[i]->setup( cm, i );
  }

  //object 配置test
  //  m_area[0]->entryObj( 0, new ObjBox() );

  m_homex = 0;

#if defined( DBG_MAP )
  DBGout();
#endif


  delete cm;
}

void Map::draw()
{
  getCurArea()->draw();
}


/*
 * 指定の object id を持つ object を探す
 * 最初に見つかったものを返す。複数あるものを順次見つける場合は、同じ OBJFINDER を使って呼び続ける。
 * objid に OBJID_ALL を指定すると全ての object を拾う
 */
ObjBase* Map::findObject( OBJFINDER& of )
{
  if( of.area < 0 || of.objidx < 0 ) {
    //初回
    of.area = of.objidx = 0;
  } else {
    //２回目以降
    of.objidx++;
  }

  for(;;) {
    if( of.area >= m_areacnt ) break;
    Area* a = m_area[ of.area ];

    for( ; of.objidx < MAX_OBJECT; of.objidx++ ) {
      ObjBase* obj = a->getObj( of.objidx );
      if( obj ) {
        if( (of.objid == OBJID_ALL) || (obj->getID() == of.objid) ) {
          return obj;
        }
      }
    }    
    of.area++;
    of.objidx = 0;
  }

  return NULL; //無かった 
}


/*
 * 別フロアへの移動
 * descend true:降りてきた==登る階段のある場所へ   false:登ってきた==降りる階段のある場所へ
 */
void Map::enterFloor( bool descend )
{
  OBJFINDER of( descend ? OBJID_UPSTAIR : OBJID_DOWNSTAIR );

  ObjBase* obj = findObject( of );

  if( !obj ) {
    return; //x!x! こうなったらおかしい… assert()
  }

  int8_t area = of.area;
  int8_t blk = 0;
  int16_t x, y;

  //入るエリアの BG マップ作成
  m_area[ area ]->makeBG( m_areaBG );

  //階段の位置に出現
  x = obj->getX() + 6;
  y = (BLKTILEH - 2) * TILEH + (TILEH / 2);

  plSetPos( x, y );

  //プレイヤーが中心に来る様にホーム修正
  m_homex = (x + 4) - (80 / 2);

  m_curareaidx = area;
}

void Map::enter( int8_t area, int8_t blk )
{
  int16_t x, y;

  //入るエリアの BG マップ作成
  m_area[ area ]->makeBG( m_areaBG );

  m_area[ area ]->getEnterPos( blk, x, y );

  plSetPos( x, y );

  //プレイヤーが中心に来る様にホーム修正
  m_homex = (x + 4) - (80 / 2);

  m_curareaidx = area;
}

void Map::update()
{
  //現在のエリアのみ更新
  getCurArea()->update();
}

//指定位置(エリア上の座標)のBGを取得
//現在表示中のエリアの値をとる。
uint8_t Map::getMapBG( int16_t x, int16_t y )
{
  int8_t bgx, bgy, block;

  bgx = x / TILEW;
  bgy = y / TILEH;
  block = bgx / BLKTILEW;
  bgx %= BLKTILEW;
  uint8_t ret = m_areaBG[ block * BLKTILEW * BLKTILEH + bgx + bgy * BLKTILEW ];

#if 0
  {
    gb.display.setColor( ColorIndex::gray );
    gb.display.setCursor( 0, 8 );
    char s[64];
    sprintf( s, PSTR("A:%d [%d,%d] %d"), area, bgx, bgy, ret );
    gb.display.print( s );
  }
#endif

  return ret;
}


Block* Map::getBlock( int16_t x, int16_t y )
{
  int8_t bgx, block;

  bgx = x / TILEW;
  block = bgx / BLKTILEW;
  return getCurArea()->getBlock( block );
}

#if defined( DBG_MAP )
void Map::DBGout()
{
  char s[256];
  TRACE( "==========" );
  for ( int ai = 0; ai < m_areacnt; ai++ ) {
    TRACE( "---------------------------------" );
    Area* a = m_area[ai];
    sprintf( s, "A:%d/%d  B:%d", ai, m_areacnt, a->m_blkcnt );
    TRACE( s );
    /*
      |-----------------|
      |    C(00,00)     |
      |C(00,00) C(00,00)|
      |    C(00,00)     |
      |-----------------|
    */

    //|----
    for ( int i = 0; i < a->m_blkcnt; i++ ) {
      TRACEN( "|-----------------" );
    }
    TRACE( "|" );

    //idx
    for ( int i = 0; i < a->m_blkcnt; i++ ) {
      sprintf( s, "|ID:%02d            ", i  );
      TRACEN( s );
    }
    TRACE( "|" );

    //Far
    for ( int i = 0; i < a->m_blkcnt; i++ ) {
      Block* b = a->m_blk[i];
      switch ( b->m_dirinfo[BDIR_FAR] ) {
        case BDINFO_CORRIDOR:
          sprintf( s, "|    C(%02d_%02d)     ", b->m_bdidata[BDIR_FAR].corridor.toArea, b->m_bdidata[BDIR_FAR].corridor.toBlock );
          TRACEN( s );
          break;
        case BDINFO_DOOR:
          sprintf( s, "|    +(%02d_%02d)     ", b->m_bdidata[BDIR_FAR].door.toArea, b->m_bdidata[BDIR_FAR].door.toBlock );
          TRACEN( s );
          break;
        case BDINFO_WALL:
          TRACEN( "|       W         " );
          break;
        default:
          TRACEN( "|                 " );
          break;
      }
    }
    TRACE( "|" );

    //LR
    for ( int i = 0; i < a->m_blkcnt; i++ ) {
      Block* b = a->m_blk[i];
      TRACEN( "|" );
      switch ( b->m_dirinfo[BDIR_LEFT] ) {
        case BDINFO_CORRIDOR:
          sprintf( s, "C(%02d_%02d)", b->m_bdidata[BDIR_LEFT].corridor.toArea, b->m_bdidata[BDIR_LEFT].corridor.toBlock );
          TRACEN( s );
          break;
        case BDINFO_DOOR:
          sprintf( s, "+(%02d_%02d)", b->m_bdidata[BDIR_LEFT].door.toArea, b->m_bdidata[BDIR_LEFT].door.toBlock );
          TRACEN( s );
          break;
        case BDINFO_WALL:
          TRACEN( "   W    " );
          break;
        default:
          TRACEN( "        " );
          break;
      }
      TRACEN( " " );
      switch ( b->m_dirinfo[BDIR_RIGHT] ) {
        case BDINFO_CORRIDOR:
          sprintf( s, "C(%02d_%02d)", b->m_bdidata[BDIR_RIGHT].corridor.toArea, b->m_bdidata[BDIR_RIGHT].corridor.toBlock );
          TRACEN( s );
          break;
        case BDINFO_DOOR:
          sprintf( s, "+(%02d_%02d)", b->m_bdidata[BDIR_RIGHT].door.toArea, b->m_bdidata[BDIR_RIGHT].door.toBlock );
          TRACEN( s );
          break;
        case BDINFO_WALL:
          TRACEN( "   W    " );
          break;
        default:
          TRACEN( "        " );
          break;
      }
    }
    TRACE( "|" );

    //Near
    for ( int i = 0; i < a->m_blkcnt; i++ ) {
      Block* b = a->m_blk[i];
      switch ( b->m_dirinfo[BDIR_NEAR] ) {
        case BDINFO_CORRIDOR:
          sprintf( s, "|    C(%02d_%02d)     ", b->m_bdidata[BDIR_NEAR].corridor.toArea, b->m_bdidata[BDIR_NEAR].corridor.toBlock );
          TRACEN( s );
          break;
        case BDINFO_DOOR:
          sprintf( s, "|    +(%02d_%02d)     ", b->m_bdidata[BDIR_NEAR].corridor.toArea, b->m_bdidata[BDIR_NEAR].corridor.toBlock );
          TRACEN( s );
          break;
        case BDINFO_WALL:
          TRACEN( "|       W         " );
          break;
        default:
          TRACEN( "|                 " );
          break;
      }
    }
    TRACE( "|" );

    //|----
    for ( int i = 0; i < a->m_blkcnt; i++ ) {
      TRACEN( "|-----------------" );
    }
    TRACE( "|" );
  }

}
#endif


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
CellMaker::CellMaker()
{
  memset( m_cell, 0, sizeof(m_cell) );
}

CellMaker::~CellMaker()
{
}

void CellMaker::make()
{
  /*
     A000B
     #####
     のAとBをつないでワープ通路とかあり？
  */

#if 0
  {
    m_areacnt = 1;
    m_cell[0].id = 1;
    m_cell[0].dist = 1;

    m_areabase[0].sx = 0;
    m_areabase[0].sy = 0;
    m_areabase[0].dir = DIR_N;
    m_areabase[0].len = 1;


    return;
  }
#endif


  m_areacnt = 3 + random(MAX_AREA - 3); //試行する回数
  uint8_t id = 1;
  int8_t sx = random(TMAPW);
  int8_t sy = random(TMAPH);
  uint8_t dir = random(DIRMAX);
  for ( uint8_t i = 0; i < m_areacnt; i++ ) {
    AREABASE* abp = &m_areabase[ id - 1];
    uint8_t len = 1 + random(MAX_BLOCK); //１エリア内の最大ブロック数まで
    abp->sx = sx; //エリアの開始位置保存
    abp->sy = sy;
    abp->dir = dir; //エリアを伸ばす方向

    //ラインを引けるだけ引く。範囲外や、他のラインに接触したら終了
    abp->len = 0;
    for ( uint8_t j = 0; j < len; j++ ) {
      CELL* cp = &m_cell[ sx + sy * TMAPW ];
      cp->id = id;
      cp->dist = abp->len;
      abp->len++;
      sx += getDirX( dir );
      sy += getDirY( dir );
      if ( sx < 0 || sx >= TMAPW || sy < 0 || sy >= TMAPH ) {
        TRACE( "over" );  //範囲外
        break;
      }
      if ( m_cell[ sx + sy * TMAPW ].id != 0 ) {
        TRACE( "hit" );  //別のエリアにぶつかる
        break;
      }
    }
    {
#if defined( DBG_MAP )
      {
        for ( int i = 0; i < TMAPH; i++ ) {
          char s[20];
          sprintf( s, "%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d",
          m_cell[ 0 + i * TMAPW ].id,
          m_cell[ 1 + i * TMAPW ].id,
          m_cell[ 2 + i * TMAPW ].id,
          m_cell[ 3 + i * TMAPW ].id,
          m_cell[ 4 + i * TMAPW ].id,
          m_cell[ 5 + i * TMAPW ].id,
          m_cell[ 6 + i * TMAPW ].id,
          m_cell[ 7 + i * TMAPW ].id,
          m_cell[ 8 + i * TMAPW ].id,
          m_cell[ 9 + i * TMAPW ].id );
          TRACE( s );
        }
        TRACE( "----------" );
      }
#endif
    }

    //接続部分情報初期化
    initConnect( id );

    //次のエリアの開始位置を決める
    //調べるエリアのリストを作成
    int8_t donecnt = id; //id は 1 から始まってる
    uint8_t* done = new uint8_t[ donecnt ];
    for ( uint8_t i = 0; i < donecnt; i++ ) done[i] = i + 1;
    gamemain.shuffle( done, donecnt ); //調べる順番をランダム化
    //現在のエリアを一番最初に持ってくる
    for ( uint8_t i = 0; i < donecnt; i++ ) {
      if ( done[i] == id ) {
        uint8_t t = done[0];
        done[0] = done[i];
        done[i] = t;
        break;
      }
    }

    //最後のエリアは次を探さない
    if ( i < m_areacnt - 1 ) {
      //現在のエリアに接する周辺のどこかから始める
      uint8_t sa;
      for (sa = 0; sa < donecnt; sa++ ) {
        uint8_t cid = done[sa];

        int8_t nx, ny, cx, cy;
        uint8_t ndir;
        if ( findNextStart( cid, cx, cy, ndir, nx, ny ) ) {
          id++; //id を進めておく

          //x!x! ここでドアとかもやる様にする？エリアに変換するときにする？
          uint8_t ct = E_CORRIDOR;
          if ( random(100) < 50 )  ct = E_DOOR;

          CELL* cc = &m_cell[ cx + cy * TMAPW ];
          cc->entrance |= ct << (ndir * 2); //進む方向を通路にする。
          //接続先情報を設定しておく
          cc->toArea[ndir] = id - 1; //この時点で ID は次のエリアの ID になっている & Area は 0 からなので -1
          cc->toBlock[ndir] = 0; //開始地点にリンクするので距離は 0

          //新しい開始地点の移動元側も通路にしておく
          uint8_t rdir = (ndir + 2) & 0x3;
          CELL* ncc = &m_cell[ nx + ny * TMAPW ];
          ncc->entrance |= ct << (rdir * 2);
          ncc->toArea[rdir] = cc->id - 1; //元いた場所の ID を Area にしてセット
          ncc->toBlock[rdir] = cc->dist;

          sx = nx;
          sy = ny;
          dir = ndir;

          break;
        }
      }
      if ( sa == donecnt ) {
        //何処にも新しい開始地点を作れなかった
        TRACE( "start point not found" );
      }
    }
    //周辺に次の開始位置が設定できないので、次のエリアの周りを調べる
    delete[] done;
  }

  //階段設置
  //down
  randomObject( O_DOWNSTAIR );
  randomObject( O_UPSTAIR );
  

  

#if defined( DBG_MAP )
  {
    for ( int i = 0; i < TMAPH; i++ ) {
      char s[20];
      sprintf( s, "%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d",
               m_cell[ 0 + i * TMAPW ].id,
               m_cell[ 1 + i * TMAPW ].id,
               m_cell[ 2 + i * TMAPW ].id,
               m_cell[ 3 + i * TMAPW ].id,
               m_cell[ 4 + i * TMAPW ].id,
               m_cell[ 5 + i * TMAPW ].id,
               m_cell[ 6 + i * TMAPW ].id,
               m_cell[ 7 + i * TMAPW ].id,
               m_cell[ 8 + i * TMAPW ].id,
               m_cell[ 9 + i * TMAPW ].id );
      TRACE( s );
    }
    TRACE( "----------" );
  }

  /*
    |-----------------|
    |ID:00 D:00/00    |
    |    C(00,00)     |
    |C(00,00) C(00,00)|
    |    C(00,00)     |
    |-----------------|
  */
  char s[128];
  static const char ec[] = { 'W', 'C', '-', '+' };
  static const char dn[] = { 'N', 'E', 'S', 'W' };
  for ( int i = 0; i < TMAPH; i++ ) {
    CELL* cp = &m_cell[0 + i * TMAPW];
    //|----
    for ( int j = 0; j < TMAPW; j++ ) {
      TRACEN( "|-----------------" );
    }
    TRACE( "|" );

    //ID, DIST
    for ( int j = 0; j < TMAPW; j++ ) {
      uint8_t id = (cp + j)->id;
      if ( id != 0 ) {
        sprintf( s, "|ID:%02d D:%02d/%02d %c  ", id - 1, (cp + j)->dist, m_areabase[id - 1].len, dn[m_areabase[id - 1].dir ] );
        TRACEN( s );
      } else {
        TRACEN( "|                 " );
      }
    }
    TRACE( "|" );

    //N
    for ( int j = 0; j < TMAPW; j++ ) {
      if ( (cp + j)->id != 0 ) {
        int8_t e = (((cp + j)->entrance >> 0) & 0x3);
        if ( e == E_CORRIDOR || e == E_DOOR) {
          sprintf( s, "|    %c(%02d_%02d)     ", ec[ e ], (cp + j)->toArea[0], (cp + j)->toBlock[0] );
        } else {
          sprintf( s, "|    %c            ", ec[ e ] );
        }
        TRACEN( s );
      } else {
        TRACEN( "|                 " );
      }
    }
    TRACE( "|" );

    //WE
    for ( int j = 0; j < TMAPW; j++ ) {
      if ( (cp + j)->id != 0 ) {
        int8_t ew = (((cp + j)->entrance >> 6) & 0x3);
        int8_t ee = (((cp + j)->entrance >> 2) & 0x3);
        TRACEN( "|" );
        if ( ew == E_CORRIDOR || ew == E_DOOR) {
          sprintf( s, "%c(%02d_%02d)", ec[ew], (cp + j)->toArea[3], (cp + j)->toBlock[3] );
        } else {
          sprintf( s, "%c       ", ec[ew] );
        }
        TRACEN( s );
        if ( ee == E_CORRIDOR || ee == E_DOOR ) {
          sprintf( s, " %c(%02d_%02d)", ec[ee], (cp + j)->toArea[1], (cp + j)->toBlock[1] );
        } else {
          sprintf( s, " %c       ", ec[ee] );
        }
        TRACEN( s );
      } else {
        TRACEN( "|                 " );
      }
    }
    TRACE( "|" );

    //S
    for ( int j = 0; j < TMAPW; j++ ) {
      if ( (cp + j)->id != 0 ) {
        int8_t e = (((cp + j)->entrance >> 4) & 0x3);
        if ( e == E_CORRIDOR || e == E_DOOR ) {
          sprintf( s, "|    %c(%02d_%02d)     ", ec[ e ], (cp + j)->toArea[2], (cp + j)->toBlock[2] );
        } else {
          sprintf( s, "|    %c            ", ec[ e ] );
        }
        TRACEN( s );
      } else {
        TRACEN( "|                 " );
      }
    }
    TRACE( "|" );
  }
  //|----
  for ( int j = 0; j < TMAPW; j++ ) {
    TRACEN( "|-----------------" );
  }
  TRACE( "|" );
#endif
}

bool CellMaker::randomObject( int8_t obj )
{
  //開始位置をランダム決定
  uint8_t s = random(TMAPW*TMAPH);

  //有効な場所を見つけるまで右下方向に向けて調べる
  //右下を超えたら左上に戻って続く
  for( uint8_t c=0; c<TMAPW*TMAPH; c++ ) {
    CELL* cell = &m_cell[s];
    if( cell->id != 0 ) {
      //有効なセル
      if( cell->mapobject == 0 ) { //まだ何も object が無い場所なら OK
        cell->mapobject = obj;
        return true;        
      }
    }
    if( ++s >= TMAPW*TMAPH ) s = 0;
  }
}

void CellMaker::initConnect( uint8_t id )
{
  AREABASE *abp = &m_areabase[ id - 1];

  //初期状態では、entrance は 0 (==全方向壁)

  int8_t sx = abp->sx;
  int8_t sy = abp->sy;
  uint8_t dir = abp->dir;

  //長さ１の場合は全方向壁のままで良し
  if ( abp->len == 1 ) return;

  //開始地点は伸ばして行く方向のみ空白
  m_cell[ sx + sy * TMAPW ].entrance |= E_NOTHING << (dir * 2);
  sx += getDirX( dir );
  sy += getDirY( dir );

  if ( abp->len >= 3 ) { //開始地点と終了地点以外がある場合
    //間は左右が通路
    for ( int i = 1; i < abp->len - 1; i++ ) {
      if ( dir & 1 ) {
        //EW
        m_cell[ sx + sy * TMAPW ].entrance |= (E_NOTHING << 6) | (E_NOTHING << 2); //EW 両方空白
      } else {
        //NS
        m_cell[ sx + sy * TMAPW ].entrance |= (E_NOTHING << 4) | (E_NOTHING << 0); //NS 両方空白
      }
      sx += getDirX( dir );
      sy += getDirY( dir );
    }
  }

  //終了地点は伸ばす方向と逆が空白
  m_cell[ sx + sy * TMAPW ].entrance |= E_NOTHING << (((dir + 2) & 3) * 2);
}

bool CellMaker::findNextStart( uint8_t curid, int8_t& x, int8_t& y, uint8_t& dir, int8_t& nx, int8_t& ny )
{
  AREABASE *abp = &m_areabase[ curid - 1];

  uint8_t len = abp->len;

  uint8_t cdist = random( len ); //調べ始める点を決める

  for (; len; len-- ) {
    uint8_t cdir = random( DIRMAX ); //調べ始める方向
    int8_t cx = abp->sx + getDirX( abp->dir ) * cdist;
    int8_t cy = abp->sy + getDirY( abp->dir ) * cdist;
    CELL* cp = &m_cell[ cx + cy * TMAPW ];
    //調べる場所の４方向を、入り口が作成可能か判定する
    int8_t i;
    int8_t tx, ty;
    for ( i = 0; i < DIRMAX; i++ ) {
      tx = cx + getDirX( cdir );
      ty = cy + getDirY( cdir );
      uint8_t mask = 0x03 << (cdir * 2);

      if ( (tx >= 0 && ty >= 0 && tx < TMAPW && ty < TMAPH) &&  //範囲外に出る場合はダメ
           (m_cell[tx + ty * TMAPW].id == 0) && //既に別エリアがあればダメ
           ((cp->entrance & mask) == 0) //その方向は既に使用中ならダメ
         ) {
        break;
      }
      cdir = (cdir + 1) & 0x3;
    }

    if ( i < DIRMAX ) {
      //見つけた
      x = cx; //現在のエリア側の位置
      y = cy;
      dir = cdir;
      nx = tx; //新しい開始位置
      ny = ty;
      return true;
    }

    //どの方向もダメなので次のセル
    if ( ++cdist >= abp->len ) cdist = 0;
  }

  //どこにも場所が無い
  return false;
}


//指定エリアの開始位置から、指定された距離にある CELL を返す
CellMaker::CELL* CellMaker::getCellFromAreaDist( uint8_t area, uint8_t dist )
{
  AREABASE* abp = &m_areabase[ area ];

  if ( dist >= abp->len ) dist = abp->len - 1;

  int8_t x = abp->sx + getDirX( abp->dir ) * dist;
  int8_t y = abp->sy + getDirY( abp->dir ) * dist;

  return &m_cell[ x + y * TMAPW ];
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


