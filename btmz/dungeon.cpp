#include "btmz.h"

#include "dungeon.h"

#include "pic.h"

#include "player.h"

#include "enemy.h"



//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Map* g_map = NULL;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void dunInit()
{
  if ( g_map ) {
    delete g_map;
  }
  g_map = new Map;
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
  BG chip

  BGidx(絵のindex。絵は同じでも attribute が異なる場合がある), Attr

  ここまで細かく設定出来なくても良い気もするけど、後でやりたくなった時の為にやっておく

  x!x! 左右反転使えばもっと少なくて済むけどどうしよう
*/
static const  uint8_t g_BGchip[] PROGMEM = {
  0, 0, //0:空白
  1, (BGATTR_BLOCK | BGATTR_PUTOBJ | BGATTR_PUTENEMY), //1:奥側壁
  2, (BGATTR_ENTER), //2:奥通路左上
  3, (BGATTR_ENTER), //3:奥通路左中
  2, (BGATTR_ENTER|BGATTR_FLIPV), //4:奥通路左下
  2, (BGATTR_ENTER|BGATTR_FLIPH), //5:奥通路右上
  3, (BGATTR_ENTER|BGATTR_FLIPH), //6:奥通路右中
  2, (BGATTR_ENTER|BGATTR_FLIPHV), //7:奥通路右下
  4, (BGATTR_BLOCK), //8:左壁一番上
  5, (BGATTR_BLOCK), //9:左壁上
  6, (BGATTR_BLOCK), //10:左壁中
  5, (BGATTR_BLOCK|BGATTR_FLIPV), //11:左壁下
  4, (BGATTR_BLOCK|BGATTR_FLIPV), //12:左壁一番下
  4, (BGATTR_BLOCK|BGATTR_FLIPH), //13:右壁一番上
  5, (BGATTR_BLOCK|BGATTR_FLIPH), //14:右壁上
  6, (BGATTR_BLOCK|BGATTR_FLIPH), //15:右壁中
  5, (BGATTR_BLOCK|BGATTR_FLIPHV), //16:右壁下
  4, (BGATTR_BLOCK|BGATTR_FLIPHV), //17:右壁一番下
  7, (BGATTR_ENTER), //18:奥ドア左上
  8, (BGATTR_ENTER), //19:奥ドア左下
  9, (BGATTR_ENTER), //20:奥ドア右上
  10, (BGATTR_ENTER), //21:奥ドア右下
  11, (BGATTR_ENTER), //22:手前ドア一番左
  12, (BGATTR_ENTER), //23:手前ドア左
  12, (BGATTR_ENTER|BGATTR_FLIPH), //24:手前ドア右
  11, (BGATTR_ENTER|BGATTR_FLIPH), //25:手前ドア一番右
  13, (BGATTR_ENTER), //26:左ドア上
  14, (BGATTR_ENTER), //27:左ドア中
  15, (BGATTR_ENTER), //28:左ドア下
  13, (BGATTR_ENTER|BGATTR_FLIPH), //29:右ドア上
  14, (BGATTR_ENTER|BGATTR_FLIPH), //30:右ドア中
  15, (BGATTR_ENTER|BGATTR_FLIPH), //31:右ドア下

  0, (BGATTR_ENTER), //32:左右通路時の通路の空白
  16, (BGATTR_BLOCK), //33:左通路時の壁
  17, (BGATTR_BLOCK), //34:右通路時の壁

  0, (BGATTR_ENTER), //35:下側通路(上下はエリア切り替え時の移動だけなので、ENTERの付いた空白が必要)
  1, (BGATTR_BLOCK|BGATTR_PUTENEMY), //36:手前壁(こいつは PUTOBJ がついてない)
  2, (BGATTR_ENTER|BGATTR_FLIPV), //37:手前通路左
  2, (BGATTR_ENTER|BGATTR_FLIPHV), //38:手前通路右
};

/*
   BG Parts
   x, y, w, h, data(bgchipidx) ...,
*/
static const uint8_t farWall[] PROGMEM = { //奥の壁
  0, 0, //x, y
  6, 5, //w, h
  0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0,
};
static const uint8_t farCorridor[] PROGMEM = { //奥の通路
  0, 0, //x, y
  6, 5, //w, h
  0, 0, 0, 0, 0, 0,
  1, 1, 2, 5, 1, 1,
  1, 1, 3, 6, 1, 1,
  1, 1, 4, 7, 1, 1,
  0, 0, 0, 0, 0, 0,
};
static const uint8_t farDoor[] PROGMEM = { //奥のドア x!x! ドア部分だけ後から書くようにした方がいいかも
  0, 0, //x, y
  6, 5, //w, h
  0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1,
  1, 1, 18, 20, 1, 1,
  1, 1, 19, 21, 1, 1,
  0, 0, 0, 0, 0, 0,
};


static const uint8_t nearWall[] PROGMEM = { //手前の壁
  0, 5, //x, y
  6, 1, //w, h
  36, 36, 36, 36, 36, 36,
};
static const uint8_t nearCorridor[] PROGMEM = { //手前の通路
  0, 5, //x, y
  6, 1, //w, h
  36, 37, 35, 35, 38, 36,
};
static const uint8_t nearDoor[] PROGMEM = { //手前のドア
  0, 5, //x, y
  6, 1, //w, h
  36, 22, 23, 24, 25, 36,
};

static const uint8_t leftWall[] PROGMEM = { //左の壁
  0, 0,
  1, 5,
  8, 9, 10, 11, 12,
};
static const uint8_t leftCorridor[] PROGMEM = { //左通路
  0, 1,
  1, 5,
  33, 33, 33, 32, 33,
};
static const uint8_t leftDoor[] PROGMEM = { //左のドア
  0, 0,
  1, 5,
  8, 9, 26, 27, 28,
};

static const uint8_t rightWall[] PROGMEM = { //右の壁
  5, 0,
  1, 5,
  13, 14, 15, 16, 17,
};
static const uint8_t rightCorridor[] PROGMEM = { //右通路
  5, 1,
  1, 5,
  34, 34, 34, 32, 34,
};
static const uint8_t rightDoor[] PROGMEM = { //右のドア
  5, 0,
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

  //絶対必要なものを先にやる
  //map object
  switch ( cell->mapobject ) {
    case CellMaker::O_UPSTAIR:
      {
        ObjUpStair* o = static_cast<ObjUpStair*>( area->createObj( OBJID_UPSTAIR ) );
        if ( o ) {
          setObjCeiling( o );
        }
      }
      break;
    case CellMaker::O_DOWNSTAIR:
      {
        ObjDownStair* o = static_cast<ObjDownStair*>( area->createObj( OBJID_DOWNSTAIR ) );
        if ( o ) {
          setObjGround( o );
        }
      }
      break;
  }


  //--- attribute 処理
#if 0
  uint8_t mapfloor = DUNMAP()->getMapFloor();
  //dark
  if ( !(cell->attr & CellMaker::AATTR_DARK) ) {
    //暗闇じゃなかったら明かり
    ObjBase* o = area->createObj( OBJID_TORCH );
    if ( o ) {
      setObjWall( o ); //壁に配置
    }

    //ろうそくも有りにする？暗闇も偶にろうそくとか置く？
    //ObjBase* o = createObj( OBJID_CANDLE );
    //m_blk[i]->setObjGround( o );  //床に配置
  }
#endif

  
#if 0
  //itemdrop (総数をカウントしておいて、全然無ければ最後に適当にばらまく？）
  if ( cell->attr & CellMaker::AATTR_ITEMDROP ) {
    ObjDropItem* o = static_cast<ObjDropItem*>( area->createObj( OBJID_DROPITEM ) );
    if ( o ) {
      ITEM* item = itGenerateFloor( mapfloor );
      o->attachItem( item );
      setObjCenter( o ); //通路上に置く
    }
  }

  //treasure
  if ( cell->attr & CellMaker::AATTR_TREASURE ) {
    //部屋によっては table とか別のコンテナにする？
    ObjBase* o = area->createObj( OBJID_CHEST );
    if ( o ) {
      area->setupContainer( static_cast<ObjContainer*>(o), mapfloor, 0 ); //中身を入れる
      setObjGround( o );
    }
  }
#endif

}

Block::~Block()
{
}


void Block::setInfo( uint8_t dir, BlockDirInfo info )
{
  m_dirinfo[dir] = info;
}

void Block::writeBGparts( Area* area, const uint8_t* parts, uint8_t* out, uint8_t* attr )
{
  if ( !parts ) return;

  const CellMaker::ROOMDATA* rd = &CellMaker::m_roomdata[ area->getRoomType() ];

  uint8_t x, y, w, h;
  x = *parts++;
  y = *parts++;
  w = x + *parts++;
  h = y + *parts++;
  for ( int i = y; i < h; i++ ) {
    for ( int j = x; j < w; j++ ) {
      uint16_t bg = *parts++;
      uint8_t frm = g_BGchip[bg * 2 + 0];

      
      if ( (frm == 1) && //壁を違うパターンと置き換えてみるテスト
           ((gamemain.randxs32()%100) < rd->brokenrate)
         ) {
        frm = 18; //壊れ壁
      }
      
      out[ j + i * TILELINEW ] = frm;
      attr[ j + i * TILELINEW ] = g_BGchip[ bg * 2 + 1 ];
    }
  }
}

/*
 * bg ... 描画するブロックの左上ブロック
 * x, y ... 描画するブロックの左上ブロックの座標
 */
void Block::draw( const uint8_t* bg, const uint8_t* attr, int16_t x, int16_t y )
{
  int16_t tx, ty, dx, dy;

  for ( int i = 0; i < BLKTILEH; i++ ) {
    for ( int j = 0; j < BLKTILEW; j++ ) {
      uint16_t idx = j + i*TILELINEW;
      uint8_t a = attr[ idx ];
      getPic( PIC_DUNGEONBG )->setFrame( bg[ idx ] );
      gb.display.drawImage( x + TILEW * j, y + TILEH * i,
                            *getPic( PIC_DUNGEONBG ),
                            (a & BGATTR_FLIPH) ? -TILEW : TILEW,
                            (a & BGATTR_FLIPV) ? -TILEH : TILEH
                            );
    }
  }
}

bool Block::isWall( uint8_t dir )
{
  return m_dirinfo[ dir ] == BDINFO_WALL;
}

void Block::makeBG( Area* area, uint8_t* out, uint8_t* aout )
{
  const uint8_t* bg;

  //奥
  switch ( m_dirinfo[BDIR_FAR] ) {
    case BDINFO_WALL: bg = farWall; break;
    case BDINFO_CORRIDOR: bg = farCorridor; break;
    case BDINFO_DOOR: bg = farDoor; break;
  }
  writeBGparts( area, bg, out, aout );

  //手前
  switch ( m_dirinfo[BDIR_NEAR] ) {
    case BDINFO_WALL: bg = nearWall; break;
    case BDINFO_CORRIDOR: bg = nearCorridor; break;
    case BDINFO_DOOR: bg = nearDoor; break;
  }
  writeBGparts( area, bg, out, aout );

  //左右通路が手前も上書くので左右は最後にやる

  //左
  switch ( m_dirinfo[BDIR_LEFT] ) {
    case BDINFO_WALL: bg = leftWall; break;
    case BDINFO_CORRIDOR: bg = leftCorridor; break;
    case BDINFO_DOOR: bg = leftDoor; break;
  }
  writeBGparts( area, bg, out, aout );

  //右
  switch ( m_dirinfo[BDIR_RIGHT] ) {
    case BDINFO_WALL: bg = rightWall; break;
    case BDINFO_CORRIDOR: bg = rightCorridor; break;
    case BDINFO_DOOR: bg = rightDoor; break;
  }
  writeBGparts( area, bg, out, aout );
}

/*
    指定の object を壁に配置する
*/
void Block::setObjWall( ObjBase* obj )
{
  int16_t x, y;
  uint8_t bx, by;
//  bool center = false;
  by = 2;
  if ( isWall( BDIR_FAR ) ) { //奥側が壁なら中央に配置
    bx = (BLKTILEW / 2 - 1);
//    center = true;
  } else {
    //通路・ドアなら左右のどっちかに配置。両方もあり？
    //  1  4
    // ==||==
    bx = (random(100) & 1) ? (BLKTILEW / 2 - 2) : (BLKTILEW / 2 + 1);
  }

  x = ( m_dist * BLKTILEW + bx) * TILEW + TILEW/2; //(center ? (TILEW / 2) : 0);
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
  x = ( m_dist * BLKTILEW + (BLKTILEW / 2)) * TILEW;
  y = 3 * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );
}

/*
    指定の object を床（中央）に配置する
*/
void Block::setObjCenter( ObjBase* obj )
{
  int16_t x, y;
  x = ( m_dist * BLKTILEW + (BLKTILEW / 2)) * TILEW;
  y = 4 * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );
}

/*
    指定の object を天井に配置
*/
void Block::setObjCeiling( ObjBase* obj )
{
  int16_t x, y;
  x = ( m_dist * BLKTILEW + (BLKTILEW / 2)) * TILEW;
  y = 0 * TILEH;

  x += obj->getOfstX();
  y += obj->getOfstY();
  obj->setPos( x, y );
}

BlockDir Block::findConnectDir( int8_t area, int8_t blk )
{
  for ( uint8_t i = BDIR_FAR; i < BDIRMAX; i++ ) {
    //その方向はドアか通路ならつながっている可能性がある
    int8_t a = 1, b = -1;
    switch ( m_dirinfo[i] ) {
      case BDINFO_DOOR:
        a = m_bdidata[i].door.toArea;
        b = m_bdidata[i].door.toBlock;
        break;
      case BDINFO_CORRIDOR:
        a = m_bdidata[i].corridor.toArea;
        b = m_bdidata[i].corridor.toBlock;
        break;
    }
    if ( a == area && b == blk ) return BlockDir(i);
  }

  return BDIRMAX; //見つからなかった
}

bool Block::save( File& f )
{
  f.write( &m_dist, sizeof(m_dist) );
  for ( int8_t i = 0; i < BDIRMAX; i++ ) {
    f.write( &m_dirinfo[i], sizeof(m_dirinfo[i]) );
    f.write( &m_bdidata[i], sizeof(m_bdidata[i]) );
  }
  return true;
}

bool Block::load( File& f )
{
  f.read(&m_dist, sizeof(m_dist));
  for ( int8_t i = 0; i < BDIRMAX; i++ ) {
    f.read(&m_dirinfo[i], sizeof(m_dirinfo[i]));
    f.read(&m_bdidata[i], sizeof(m_bdidata[i]));
  }
  return true;
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
    for ( int i = 0; i < m_blkcnt; i++ ) {
      if ( m_blk[i] ) delete m_blk[i];
    }
    delete[] m_blk;
  }

  for ( int i = 0; i < MAX_OBJECT; i++ ) {
    if ( m_obj[i] ) {
      m_obj[i]->finish();
      delete m_obj[i];
    }
  }
}

void Area::setup( CellMaker* cm, uint8_t id )
{
  CellMaker::AREABASE *abp = cm->getAreaBase( id );
  m_blkcnt = abp->len;
  m_blk = new Block*[m_blkcnt];

  m_roomtype = abp->rtype;

  int8_t x, y;
  x = abp->sx;
  y = abp->sy;

  //Block 生成
  for ( uint8_t i = 0; i < m_blkcnt; i++ ) {
    CellMaker::CELL* cell = cm->getCell( x, y );
    m_blk[i] = new Block( this, cm, abp, cell );

    abp->fwdPos( x, y );
  }

  //object 配置の為に attribute が必要なので、一旦BGを書き出す
  makeBG( id, DUNMAP()->getAreaBG(), DUNMAP()->getAttrBG() );

  //壁に割り当てる object
  //torch
  if( !(abp->attr & CellMaker::AATTR_DARK) ) {
    //暗闇じゃなかったら明かりを１ブロックに１個置く
    for( uint8_t i=0; i<m_blkcnt; i++ ) {
      if( random(100) < 5 ) continue; //偶に置かない
      ObjBase* o = createObj( OBJID_TORCH );
      if( o ) {
        m_blk[i]->setObjWall( o );
        clrAttrByObj( o, BGATTR_PUTOBJ );
      }
      
      //ろうそくも有りにする？暗闇も偶にろうそくとか置く？
      //ObjBase* o = createObj( OBJID_CANDLE );
      //m_blk[i]->setObjGround( o );  //床に配置
    }
  }

  //mapobj
  {
    static const uint8_t mapobjid[] = {
      OBJID_TABLE, OBJID_TAPESTRY, OBJID_STATUE, OBJID_FOUNTAIN,
      OBJID_SKELTON, OBJID_TOMB, OBJID_SHELF, OBJID_CHAIN,
    };
    const CellMaker::ROOMDATA* rd = &CellMaker::m_roomdata[abp->rtype];
    uint8_t objtbl[ 16 ];
    uint8_t objtblsz = 0;

    //x!x! 部屋のタイプ的に絶対必用な物・１個しか置いたら駄目な物とかに対応する事
    int8_t objcnt = rd->minmapobj + random( m_blkcnt+1 );
    for( int8_t c=0; c<objcnt; c++ ) {
      objtblsz = 0;
      for( int8_t i=0; i<16; i++ ) {
        if( rd->mapobjflag & (1<<i) ) objtbl[objtblsz++] = mapobjid[i];
      }
      if( objtblsz > 0 ) {
        ObjBase* o = createObj( objtbl[ random(objtblsz) ] );
        if ( o ) {
          if ( !setObjToRandomGround( o ) ) {
            removeObj( o ); //置けなかったので無かった事に。
          }
        }
      }
    }
  }


  uint8_t mapfloor = DUNMAP()->getMapFloor();
  uint8_t bidx[ MAX_BLOCK ];

  //dropitem
  gamemain.getShuffledIdx( bidx, m_blkcnt );
  for ( uint8_t i = 0; i < abp->numitemdrop; i++ ) {
    uint8_t bi = bidx[i];

    allocDropitem( bi );
  }

  //treasure
  gamemain.getShuffledIdx( bidx, m_blkcnt );
  for ( uint8_t i = 0; i < abp->numtreasure; i++ ) {
    allocContainer();
  }

  //enemy
  //    enCreate( ENTYPE( ENTYPE_SLIME + random(ENTYPEMAX - 1) ), 0, id, 0, 0 );
  for ( uint8_t i = 0; i < abp->enemynum; i++ ) {
    allocEnemy( id );
  }


#if defined( DBG_MAP )
  //for DEBUG
  m_DBGsx = abp->sx;
  m_DBGsy = abp->sy;
  m_DBGdir = abp->dir;
#endif
}

void Area::allocDropitem( uint8_t bi )
{
  uint8_t mapfloor = DUNMAP()->getMapFloor();
  ObjDropItem* o = static_cast<ObjDropItem*>( createObj( OBJID_DROPITEM ) );
  if ( o ) {
    ITEM* item = itGenerateFloor( mapfloor );
    o->attachItem( item );
    m_blk[bi]->setObjCenter( o ); //通路上に置く
    DUNMAP()->incNumDropitem();
  }
}

void Area::allocContainer()
{
  uint8_t mapfloor = DUNMAP()->getMapFloor();
  //部屋によっては table とか別のコンテナにする？
  ObjBase* o = createObj( OBJID_CHEST );
  if ( o ) {
    //x!x! 基本横2tileあるので、１部屋だと置けない気がする。一部屋用の小さい箱用意する？
    if ( !setObjToRandomGround( o ) ) {
      removeObj( o ); //置けなかったので無かった事に。
    } else {
      setupContainer( static_cast<ObjContainer*>(o), mapfloor, 0 ); //中身を入れる
      DUNMAP()->incNumContainer();
    }
  }
}

void Area::allocEnemy( uint8_t areaid )
{
  EnemyData* ed = enCreate( ENTYPE( ENTYPE_SLIME + random(ENTYPEMAX - 1) ), 0, areaid );
  if ( !setEnemyToRandom( ed ) ) {
    enDelete( ed );
  } else {
    DUNMAP()->incNumEnemy();
  }
}

void Area::draw()
{
  int16_t hx, hy;
  hx = DUNMAP()->getHomeX();
  hy = DUNMAP()->getHomeY();

  //背景
  uint8_t* bg = DUNMAP()->getAreaBG();
  uint8_t* attr = DUNMAP()->getAttrBG();

  for ( int i = 0; i < m_blkcnt; i++ ) {
    int16_t bx, by;

    bx = -hx + i * BLKTILEW * TILEW;
    by = -hy;

    //bg ... 描画するブロックの左上ブロック
    //bx ... 描画するブロックの左上ブロックの座標
    m_blk[i]->draw( bg, attr, bx, by );

    bg += BLKTILEW;
    attr += BLKTILEW;
  }

#if 01
  //object
  //レイヤー毎に分けて描く
  //x!x! プレイヤーと敵もまとめて管理しないと駄目かも…
  for ( int8_t lyr = 0; lyr < MAX_DRAWLYR; lyr++ ) {
    for ( int8_t i = 0; i < MAX_OBJECT; i++ ) {
      if ( m_obj[i] ) {
        if ( m_obj[i]->getDrawLayer() == lyr ) {
          m_obj[i]->draw();
        }
      }
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



BlockDir Area::getEnterPos( uint8_t blk, int8_t prvarea, int8_t prvblk, int16_t& x, int16_t& y )
{
  BlockDir bd;

  //指定ブロックの中央
  x = blk * BLKTILEW * TILEW + (BLKTILEW * TILEW / 2);
  y = (BLKTILEH - 2) * TILEH + (TILEH / 2);

  //指定ブロックの、以前のブロックとつながっている方向を取得
  bd = m_blk[blk]->findConnectDir( prvarea, prvblk );

  //進入方向によってずらす
  switch ( bd ) {
    case BDIR_FAR:
      y -= ENTEROFSTV;
      break;
    case BDIR_RIGHT:
      x += ENTEROFSTH;
      break;
    case BDIR_NEAR:
      y += ENTEROFSTV;
      break;
    case BDIR_LEFT:
      x -= ENTEROFSTH;
      break;
  }

  return bd;
}

/*
   背景BGを生成して格納する。
   保持する全ブロック分を一度に作成。
*/
void Area::makeBG( uint8_t id, uint8_t* out, uint8_t* aout )
{
  //マップ用 xorshift32 乱数初期化
  gamemain.initXorshift32( DUNMAP()->getBaseSeed() + id );

  uint8_t* aoutorg;
  for ( int8_t i = 0; i < m_blkcnt; i++ ) {
    m_blk[i]->makeBG( this, out, aout );
    out += BLKTILEW;
    aout += BLKTILEW;
  }

  //x!x! マップ左右の出入り口となる場所には敵が置けないようにしないと駄目
  //x!x! 上の方は置ける様にしておく？
  //左端が出入り口になってる？
  uint8_t* ca = &aoutorg[ 0 * BLKTILEW + 3 * TILELINEW ];
  if ( *ca & BGATTR_PUTENEMY ) {
    //出入り口とその隣は敵置けない
    *ca &= ~BGATTR_PUTENEMY;
    *(ca + 1) &= ~BGATTR_PUTENEMY;
  }

  //右端が出入り口になってる？
  ca = &aoutorg[ (m_blkcnt - 1) * BLKTILEW + (BLKTILEW-1) + 3 * TILELINEW ];
  if ( *ca & BGATTR_PUTENEMY ) {
    //出入り口とその隣は敵置けない
    *ca &= ~BGATTR_PUTENEMY;
    *(ca - 1) &= ~BGATTR_PUTENEMY;
  }


  //開始位置に敵置けない（階段のある場所）
  { //上り
    Map::OBJFINDER of( OBJID_UPSTAIR );
    ObjBase* obj = DUNMAP()->findObject( of );
    if ( obj ) {
      DUNMAP()->clrAttrBGByTile( obj->getX(), 3 * TILEH, BGATTR_PUTENEMY );
    }
  }
  { //下り
    Map::OBJFINDER of( OBJID_DOWNSTAIR );
    ObjBase* obj = DUNMAP()->findObject( of );
    if ( obj ) {
      DUNMAP()->clrAttrBGByTile( obj->getX(), 3 * TILEH, BGATTR_PUTENEMY );
    }
  }
}

int16_t Area::getWidth()
{
  return m_blkcnt * BLKTILEW * TILEW;
}

ObjBase* Area::createObj( uint8_t objid )
{
  ObjBase* obj = createObjInstance( objid );

  if ( obj ) {
    if ( !entryObj( obj ) ) {
      //登録出来なかった
      delete obj;
      obj = NULL;
    } else {
      //登録出来た
    }
  }

  return obj;
}

bool Area::entryObj( ObjBase* obj )
{
  uint8_t i;
  for ( i = 0; i < MAX_OBJECT; i++ ) {
    if ( !m_obj[i] ) break;
  }
  if ( i == MAX_OBJECT ) return false; //もう登録出来ない

  m_obj[i] = obj;
  obj->setUID( i );
//  obj->setBlock( blk );
  obj->init();

  return true;
}

ObjBase* Area::createObjInstance( uint8_t objid )
{
  ObjBase* obj = NULL;
  switch ( objid ) {
    case OBJID_TORCH: obj = new ObjTorch(); break;
    case OBJID_CANDLE: obj = new ObjCandle(); break;
    case OBJID_CHEST: obj = new ObjChest(); break;
    case OBJID_TABLE: obj = new ObjTable(); break;
    case OBJID_UPSTAIR: obj = new ObjUpStair(); break;
    case OBJID_DOWNSTAIR: obj = new ObjDownStair(); break;
    case OBJID_DROPITEM: obj = new ObjDropItem(); break;
    case OBJID_TAPESTRY: obj = new ObjTapestry(); break;
    case OBJID_STATUE: obj = new ObjStatue(); break;
    case OBJID_SKELTON: obj = new ObjSkelton(); break;
    case OBJID_TOMB: obj = new ObjTomb(); break;
    case OBJID_SHELF: obj = new ObjShelf(); break;
    case OBJID_CHAIN: obj = new ObjChain(); break;
  }

  if ( obj ) {
    obj->setID( objid );
  }

  return obj;
}


/*
 * 通路の奥側のランダムな場所に object を配置
 */
bool Area::setObjToRandomGround( ObjBase* obj )
{
  static const int8_t GRNDTY = 3;
  uint8_t* attr = DUNMAP()->getAttrBG();
  uint8_t tbl[ MAX_BLOCK * BLKTILEW ];
  uint8_t cnt = 0;

  uint8_t btmax = BLKTILEW * m_blkcnt;
  int8_t tw, th; //object のタイルサイズ
  getObjTileSize( obj, tw, th );

  //置くことが可能な場所を探す
  for ( uint8_t i = 0; i < btmax; i++ ) {
    //置くことが可能な場所を抽出
    if( isSetAttrRect( i, GRNDTY, tw, -th, BGATTR_PUTOBJ ) ) { //床から上方向に調べるので高さを -
      tbl[cnt++] = i;
    }
  }

  if( cnt > 0 ) {
    //置ける場所があった。
    uint8_t ti = tbl[ random( cnt ) ];
    int16_t x = ti * TILEW + (tw*TILEW) / 2;
    x += obj->getOfstX();
    obj->setPos( x, GRNDTY * TILEH + obj->getOfstY() );

    clrAttrRect( ti, GRNDTY, tw, -th, BGATTR_PUTOBJ );

    return true;
  }

  return false;
}

/*
 * object が占めるタイルのサイズを取得
 */
void Area::getObjTileSize( ObjBase* obj, int8_t& tw, int8_t& th )
{
  Image* img = getPic( obj->getPicID() );
  int16_t w, h;
  w = img->width();
  h = img->height();

  tw = w / TILEW;
  if( w % TILEW ) tw++;
  th = h / TILEH;
  if( h % TILEH ) th++;
}

/*
 * 指定の矩形の中に、指定の BG attr が全て存在しているか調べる
 */
bool Area::isSetAttrRect( uint8_t x, uint8_t y, int8_t w, int8_t h, uint8_t attr )
{
  uint8_t x1 = x + w;
  uint8_t y1 = y + h;
  if( x1 < x ) {
    uint8_t t = x;
    x = x1;
    x1 = t;
  }
  if( y1 < y ) {
    uint8_t t = y;
    y = y1;
    y1 = t;
  }
  for( ; y<y1; y++ ) {
    for( uint8_t tx=x; tx<x1; tx++ ) {
      if( !(DUNMAP()->getAttrBGByTile( tx, y ) & attr) ) return false; //無い
    }
  }
  return true;
}

/*
 * 指定の矩形の中の指定の BG attr を全てクリア
 */
void Area::clrAttrRect( uint8_t x, uint8_t y, int8_t w, int8_t h, uint8_t attr )
{
  uint8_t x1 = x + w;
  uint8_t y1 = y + h;
  if( x1 < x ) {
    uint8_t t = x;
    x = x1;
    x1 = t;
  }
  if( y1 < y ) {
    uint8_t t = y;
    y = y1;
    y1 = t;
  }
  for( ; y<y1; y++ ) {
    for( uint8_t tx=x; tx<x1; tx++ ) {
      DUNMAP()->clrAttrBGByTile( tx, y, attr );
    }
  }
}

void Area::clrAttrByObj( ObjBase* obj, uint8_t attr )
{
  uint8_t x0 = obj->getX() / TILEW;
  uint8_t y0 = obj->getY() / TILEH;
  int8_t tw, th;
  getObjTileSize( obj, tw, th );
  clrAttrRect( x0, y0, tw, th, attr );
}

bool Area::setEnemyToRandom( EnemyData* ed )
{
  static const int8_t GRNDTY = 3;
  static const int8_t NEARTY = GRNDTY + 2;
  uint8_t* attr = DUNMAP()->getAttrBG();
  uint8_t tbl[ MAX_BLOCK * BLKTILEW ];
  uint8_t cnt = 0;

  uint8_t btmax = BLKTILEW * m_blkcnt;
  for ( uint8_t i = 0; i < btmax; i++ ) {
    //置くことが可能な場所を抽出
    if ( (DUNMAP()->getAttrBGByTile(i, GRNDTY) & BGATTR_PUTENEMY) &&
         (DUNMAP()->getAttrBGByTile(i, NEARTY) & BGATTR_PUTENEMY)
       ) {
      tbl[cnt++] = i;
    }
  }
  gamemain.shuffle( tbl, cnt );

  EnemyTemplate* et = ENTPL( ed->type );

  //配置objectが占めるブロック数を計算
  int16_t w = et->w; //obj->getOfstX() * 2; //x!x! object の x 座標は中央なので、オフセットの２倍が横幅のはず。
  if ( w < 0 ) w = -w;
  uint8_t bx = (w / TILEW);
  if ( w % TILEW ) bx++;

  //横幅ブロック分連続で配置可能領域なら置ける
  for ( uint8_t i = 0; i < cnt; i++ ) {
    uint8_t ti = tbl[i];
    uint8_t x;
    for ( x = 1; x < bx; x++ ) { //ti の場所はそもそも置ける場所なので、+1 から調べる
      if ( !(DUNMAP()->getAttrBGByTile( (ti + x), GRNDTY ) & BGATTR_PUTENEMY) ||
           !(DUNMAP()->getAttrBGByTile( (ti + x), NEARTY ) & BGATTR_PUTENEMY)
         ) {
        break; //置けない場所ならダメ
      }
    }
    if ( x >= bx ) {
      //object の幅分置けるので設置
      int16_t x, y;
      x = (ti + bx / 2) * TILEW;
      x = ti * TILEW + (bx*TILEW) / 2;
      y = (GRNDTY + 1) * TILEH + (TILEH / 2);
      x += et->w / 2;

      enSetPos( ed, TOFIX(x), TOFIX(y) );


      //接地可能 attribute 削除
      for ( x = 0; x < bx; x++ ) {
        DUNMAP()->clrAttrBGByTile( (ti + x), GRNDTY, BGATTR_PUTENEMY );
        DUNMAP()->clrAttrBGByTile( (ti + x), NEARTY, BGATTR_PUTENEMY ); //上だけでいい気もする
      }

      return true;
    }
  }

  return false;
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

void Area::setupContainer( ObjContainer* objc, uint8_t mapfloor, uint8_t droplvl )
{
  //個数
  uint8_t num = random( ObjContainer::MAX_CONTENTS - 1 ) + 1;

  for ( int8_t i = 0; i < num; i++ ) {
    ObjDropItem* o = static_cast<ObjDropItem*>( createObj( OBJID_DROPITEM ) );
    if ( o ) {
      ITEM* item = itGenerateFloor( mapfloor );
      o->attachItem( item );

      //コンテナに格納
      objc->addObj( o );
    } else {
      //もう生成出来ない
      break;
    }
  }
}

bool Area::save( File& f )
{
  f.write( &m_blkcnt, sizeof(m_blkcnt));
  for ( uint8_t i = 0; i < m_blkcnt; i++ ) {
    if ( !m_blk[i]->save(f) ) return false;
  }

  for ( int8_t i = 0; i < MAX_OBJECT; i++ ) {
    if ( m_obj[i] ) {
      if ( !m_obj[i]->save(f) ) return false;
    } else {
      //無効な id, uid を書いておく
      if ( !ObjBase::saveInvalidIDs(f) ) return false;
    }
  }

  f.write( &m_roomtype, sizeof(m_roomtype));

  return true;
}

bool Area::load( File& f )
{
  //block count
  f.read( &m_blkcnt, sizeof(m_blkcnt));

#if defined( DBG_SAVELOAD )
  {
    char s[128];
    sprintf( s, "LOAD>blkcnt %d", m_blkcnt );
    TRACE( s );
  }
#endif

  //each block
  m_blk = new Block*[m_blkcnt];
  memset( m_blk, 0, sizeof(m_blk[0])*m_blkcnt );
  for (uint8_t i = 0; i < m_blkcnt; i++ ) {
    m_blk[i] = new Block();
    if ( !m_blk[i]->load(f) ) {
#if defined( DBG_SAVELOAD )
      {
        char s[80];
        sprintf( s, "LOAD>failed", i );
        TRACE( s );
      }
#endif
      return false;
    }
  }

  //object
  memset( m_obj, 0, sizeof(m_obj) );
  for ( int i = 0; i < MAX_OBJECT; i++ ) {
    uint8_t id, uid;
    if ( !ObjBase::loadIDs( f, id, uid ) ) {
#if defined( DBG_SAVELOAD )
      {
        char s[80];
        sprintf( s, "LOAD>objID failed" );
        TRACE( s );
      }
#endif
      return false;
    }
#if defined( DBG_SAVELOAD )
    {
      char s[128];
      sprintf( s, "LOAD>createobj id:%d uid:%d", id, uid );
      TRACE( s );
    }
#endif
    if ( uid == INVALID_UID ) {
      continue; //無効な場所
    } else {
      ObjBase* o = createObjInstance( id );
      if ( !o ) {
#if defined( DBG_SAVELOAD )
        {
          char s[80];
          sprintf( s, "LOAD>createObjInstance id:%d failed", id );
          TRACE( s );
        }
#endif
        return false;
      }

      o->setUID( uid );

      if ( !o->load( f ) ) {
#if defined( DBG_SAVELOAD )
        {
          char s[80];
          sprintf( s, "LOAD>obj failed" );
          TRACE( s );
        }
#endif
        delete o;
        return false;
      }
      m_obj[ uid ] = o;
    }
  }

  //全ての object 読み込み完了後、uid で保存していたポインタをポインタに戻す
  for ( int i = 0; i < MAX_OBJECT; i++ ) {
#if defined( DBG_SAVELOAD )
    {
      char s[80];
      sprintf( s, "LOAD>obj resolve %d[%08x]", i, m_obj[i] );
      TRACE( s );
    }
#endif
    if ( m_obj[i] ) {
      m_obj[i]->resolvePtr( m_obj );
    }
  }

  f.read( &m_roomtype, sizeof(m_roomtype));

  return true;
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

void Map::create( uint8_t mapfloor )
{
  m_baseseed = random( 0xffffffff );

  m_mapfloor = mapfloor;

  //仮想マップ構築
  CellMaker* cm = new CellMaker();
  cm->make( mapfloor );

  //仮想マップの情報を元にエリア作成
  m_areacnt = cm->getAreaCount();
  m_area = new Area*[m_areacnt];
  for ( int i = 0; i < m_areacnt; i++ ) {
    m_area[i] = new Area;
    m_area[i]->setup( cm, i );
  }

  //色々少なかったら追加する
  uint8_t tbl[MAX_AREA];
  uint8_t minnum, tblidx;
  //dropitem
  minnum = m_areacnt / 3;
  //絶対置いては駄目なエリアは除外する必要がある。そんな場所ある？
  gamemain.getShuffledIdx( tbl, m_areacnt );  
  tblidx = 0;
  while( getNumDropitem() < minnum ) {
    Area* a = m_area[ tbl[tblidx] ];
    a->allocDropitem( random( a->getBlockCnt() ) );
    if( ++tblidx == m_areacnt ) break; //もうどうしても置けない
  }

  //treasure
  minnum = 1;
  //絶対置いては駄目なエリアは除外する必要がある。そんな場所ある？
  gamemain.getShuffledIdx( tbl, m_areacnt );  
  tblidx = 0;
  while( getNumContainer() < minnum ) {
    Area* a = m_area[ tbl[tblidx] ];
    a->allocContainer();
    if( ++tblidx == m_areacnt ) break; //もうどうしても置けない
  }

  //x!x! 固定の敵と、エリアに出入りする度に出現するランダム湧きの敵を用意する？
  //enemy
  minnum = ((m_areacnt*2) < MAX_ENEMYENTRY) ? (m_areacnt*2) : MAX_ENEMYENTRY;
  //絶対置いては駄目なエリアは除外する必要がある。そんな場所ある？
  gamemain.getShuffledIdx( tbl, m_areacnt );  
  tblidx = 0;
  while( getNumEnemy() < minnum ) {
    Area* a = m_area[ tbl[tblidx] ];
    uint8_t cnt = 1 + random( a->getBlockCnt() );
    //x!x! 既に敵がいるエリアはその分減らす？
    for( int i=0; i<cnt; i++ ) {
      a->allocEnemy( tbl[tblidx] );
    }
    if( ++tblidx == m_areacnt ) break; //もうどうしても置けない
  }
  
  m_homex = 0;

#if defined( DBG_MAP )
  DBGout();
#endif


  delete cm;
}

void Map::draw()
{
  getCurArea()->draw();

#if defined( DBG_SHOW_ROOMNAME )
  {
    Area* a = getCurArea();
    gb.display.setColor( ColorIndex::gray );
    gb.display.setCursor( 12, 58 );
    /*
      RTYPE_CORRIDOR, //通路（部屋じゃないけど）
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
     */
    static const char* name[] = {
      "Corridor", "Barracks", "Privateroom", "Prison", "Throne",
      "Fountain", "Altar", "Cemetery", "Laboratory", "Ruin",
      "Treasure", "Hall"
    };
    gb.display.print( name[a->getRoomType()] );
  }
#endif
}


/*
   指定の object id を持つ object を探す
   最初に見つかったものを返す。複数あるものを順次見つける場合は、同じ OBJFINDER を使って呼び続ける。
   objid に OBJID_ALL を指定すると全ての object を拾う
*/
ObjBase* Map::findObject( OBJFINDER& of )
{
  if ( of.area < 0 || of.objidx < 0 ) {
    //初回
    of.area = of.objidx = 0;
  } else {
    //２回目以降
    of.objidx++;
  }

  for (;;) {
    if ( of.area >= m_areacnt ) break;
    Area* a = m_area[ of.area ];

    for ( ; of.objidx < MAX_OBJECT; of.objidx++ ) {
      ObjBase* obj = a->getObj( of.objidx );
      if ( obj ) {
        if ( (of.objid == OBJID_ALL) || (obj->getID() == of.objid) ) {
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
   別フロアへの移動
   descend true:降りてきた==登る階段のある場所へ   false:登ってきた==降りる階段のある場所へ
*/
void Map::enterFloor( bool descend )
{
  OBJFINDER of( descend ? OBJID_UPSTAIR : OBJID_DOWNSTAIR );

  ObjBase* obj = findObject( of );

  if ( !obj ) {
    return; //x!x! こうなったらおかしい… assert()
  }

  int8_t area = of.area;
  int8_t blk = 0;
  int16_t x, y;

  //入るエリアの BG マップ作成
  m_area[ area ]->makeBG( area, m_areaBG, m_attrBG );

  //階段の位置に出現
  x = obj->getX() + 6;
  y = (BLKTILEH - 2) * TILEH + (TILEH / 2);

  plSetEnterPos( x, y );

  //プレイヤーが中心に来る様にホーム修正
  m_homex = (x + 4) - (SCRW / 2);

  m_curareaidx = area;
}

BlockDir Map::enter( int8_t area, int8_t blk, int8_t prvarea, int8_t prvblk )
{
  int16_t x, y;

  //入るエリアの BG マップ作成
  m_area[ area ]->makeBG( area, m_areaBG, m_attrBG );

  BlockDir ed = m_area[ area ]->getEnterPos( blk, prvarea, prvblk, x, y );

  plSetEnterPos( x, y );

  //プレイヤーが中心に来る様にホーム修正
  m_homex = (x + 4) - (SCRW / 2);

  m_curareaidx = area;

  return ed;
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
  int8_t bgx, bgy;//, block;

  bgx = x / TILEW;
  bgy = y / TILEH;
#if 0
  block = bgx / BLKTILEW;
  bgx %= BLKTILEW;
  uint8_t ret = m_areaBG[ block * BLKTILEW * BLKTILEH + bgx + bgy * BLKTILEW ];
  return ret;
#else
  return getMapBGByTile( bgx, bgy );
#endif

}

uint8_t Map::getAttrBG( int16_t x, int16_t y )
{
  int8_t bgx, bgy;//, block;

  bgx = x / TILEW;
  bgy = y / TILEH;
#if 0
  block = bgx / BLKTILEW;
  bgx %= BLKTILEW;
  uint8_t ret = m_attrBG[ block * BLKTILEW * BLKTILEH + bgx + bgy * BLKTILEW ];
  return ret;
#else
  return getAttrBGByTile( bgx, bgy );
#endif

}


Block* Map::getBlock( int16_t x, int16_t y )
{
  int8_t bgx, block;

  bgx = x / TILEW;
  block = bgx / BLKTILEW;
  return getCurArea()->getBlock( block );
}


bool Map::save()
{
  File f = SD.open( "MAP.SAV", FILE_WRITE );
  if ( !f ) return false;

  f.seekSet( 0 );

  //version
  int32_t v = VER_SAVEDATA;
  f.write( &v, sizeof(v) );
  f.write( &m_areacnt, sizeof(m_areacnt));
  f.write( &m_homex, sizeof(m_homex));
  f.write( &m_homey, sizeof(m_homey));
  f.write( &m_curareaidx, sizeof(m_curareaidx));
  f.write( &m_baseseed, sizeof(m_baseseed) );

  for ( int8_t i = 0; i < m_areacnt; i++ ) {
    if ( !m_area[i]->save(f) ) {
      f.close();
      return false;
    }
  }

  //敵データ
  enSave( f );

  //おしまい
  f.close();

  return true;
}

bool Map::load()
{

  //x!x! 各種ロードに失敗したら、色々破棄して終わる事
  File f = SD.open( "MAP.SAV", FILE_READ );
  if ( !f ) return false;

  int32_t v;
  f.read( &v, sizeof(v));
  if ( v != VER_SAVEDATA ) return false;

  f.read( &m_areacnt, sizeof(m_areacnt));
  f.read( &m_homex, sizeof(m_homex));
  f.read( &m_homey, sizeof(m_homey));
  f.read( &m_curareaidx, sizeof(m_curareaidx));
  f.read( &m_baseseed, sizeof(m_baseseed));

#if defined( DBG_SAVELOAD )
  {
    char s[128];
    sprintf( s, "LOAD>v:%d  ac:%d  hx:%d  hy:%d  ca:%d  seed:%d", v, m_areacnt, m_homex, m_homey, m_curareaidx, m_baseseed );
    TRACE( s );
  }
#endif


  m_area = new Area*[ m_areacnt ];
  memset( m_area, 0, sizeof(m_area[0])*m_areacnt );
  for ( int8_t i = 0; i < m_areacnt; i++ ) {
    m_area[i] = new Area();
#if defined( DBG_SAVELOAD )
    {
      char s[64];
      sprintf( s, "LOAD>area %d", i );
      TRACE( s );
    }
#endif
    if ( !m_area[i]->load(f) ) {
#if defined( DBG_SAVELOAD )
      {
        char s[64];
        sprintf( s, "LOAD>area %d failed", i );
        TRACE( s );
      }
#endif
      f.close();
      return false; //失敗 x!x! 戻った後中途半端な生成物を破棄する事
      //x!x! こっちで破棄する？
    }
  }

  //BG マップを再生成
  m_area[ m_curareaidx ]->makeBG( m_curareaidx, m_areaBG, m_attrBG );

  //敵データ
  enLoad( f );

  f.close();

  return true;
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
/*
    //部屋の情報
    typedef struct {
      uint8_t minsize; //最低blockサイズ
      uint8_t brokenrate; //壊れ度(壁とか壊れた感じにする割合(0-100))
      uint8_t itemrate; //アイテムが落ちてる割合(0-100)
      uint8_t chestrate; //宝箱がある割合(0-100)
      uint8_t droplvlcorrection; //ドロップレベル補正
      uint8_t enemyrate; //敵出現率
      uint8_t minenemy; //敵最小数
      uint8_t maxenmey; //敵最大数
      uint8_t minmapobj; //最低限配置する mapobject 数
      uint16_t mapobjflag; //出現する mapobject
    } ROOMDATA;
    
      RTYPE_CORRIDOR, //通路（部屋じゃないけど）
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
*/
const CellMaker::ROOMDATA CellMaker::m_roomdata[CellMaker::MAXRTYPE] = {
  //minsize, brokenrate, itemrate, chestrate, droplvlcorrection, enemyrate, minenemy, maxenemy, minmapobj, mapobjflag
  { 1,  5,   2,   2, 0,   5, 1, 2, 0, (MOBJ_TAPESTRY|MOBJ_STATUE|MOBJ_CHAIN) }, //corridor
  { 4,  2,  15,  40, 0, 100, 3, 6, 3, (MOBJ_TAPESTRY|MOBJ_SHELF) }, //barracks
  { 1,  3,  30,  30, 0,  30, 1, 3, 2, (MOBJ_TABLE|MOBJ_TAPESTRY|MOBJ_SHELF) }, //privateroom
  { 1,  8,   5,   5, 0,  40, 1, 4, 2, (MOBJ_SKELTON|MOBJ_CHAIN) }, //prison
  { 4,  0,  10,  50, 1,  80, 2, 5, 3, (MOBJ_STATUE|MOBJ_TAPESTRY) }, //throne
  { 1,  3,  10,   5, 1,   0, 0, 0, 0, (MOBJ_NONE)},//(MOBJ_FOUNTAIN) }, //fountain
  { 1,  5,   5,  10, 0,  10, 1, 1, 0, (MOBJ_STATUE|MOBJ_TOMB) }, //altar
  { 3, 10,  50,  10, 0,  80, 1, 3, 3, (MOBJ_TOMB|MOBJ_SKELTON) }, //cemetery
  { 2,  3,  50,  50, 0,  30, 1, 2, 2, (MOBJ_TABLE|MOBJ_SHELF) }, //laboratory
  { 2, 50, 100,   0, 5, 100, 1, 1, 3, (MOBJ_STATUE) }, //ruin
  { 2, 10,   0, 100, 2,  60, 2, 5, 2, (MOBJ_TAPESTRY|MOBJ_SHELF) }, //treasure
  { 5, 10,  20,  20, 0,  75, 2, 6, 4, (MOBJ_TAPESTRY) }, //hall
};

CellMaker::CellMaker()
{
  memset( m_cell, 0, sizeof(m_cell) );
  memset( m_areabase, 0, sizeof(m_areabase) );
}

CellMaker::~CellMaker()
{
}

void CellMaker::make( uint8_t mapfloor )
{
  m_tgtfloor = mapfloor;

  /*
     A000B
     #####
     のAとBをつないでワープ通路とかあり？
  */
  m_areacnt = MIN_AREA + random(MAX_AREA - MIN_AREA); //試行する回数
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

  //部屋割当
  makeRoom();
  //部屋の状態に合わせて cell の詳細設定
  setupCell();


  //階段設置
  //down
  randomObject( O_DOWNSTAIR );
  randomObject( O_UPSTAIR );




#if defined( DBG_MAP )
  DBGdumpMap();
#endif
}


bool CellMaker::randomObject( int8_t obj )
{
  //開始位置をランダム決定
  uint8_t s = random(TMAPW * TMAPH);

  //有効な場所を見つけるまで右下方向に向けて調べる
  //右下を超えたら左上に戻って続く
  for ( uint8_t c = 0; c < TMAPW * TMAPH; c++ ) {
    CELL* cell = &m_cell[s];
    if ( cell->id != 0 ) {
      //有効なセル
      if ( cell->mapobject == 0 ) { //まだ何も object が無い場所なら OK
        cell->mapobject = obj;
        return true;
      }
    }
    if ( ++s >= TMAPW * TMAPH ) s = 0;
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


#if defined( DBG_MAP )
void DBGdumpMap()
{
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
}
#endif



void CellMaker::makeRoom()
{
  static const InitRoomFunc initRoomTbl[ MAXRTYPE ] = {
    initRoomCorridor, //corridor
    initRoomBarracks, initRoomPrivateRoom, initRoomPrison, initRoomThrone,
    initRoomFountain, initRoomAltar, initRoomCemetery, initRoomLaboratory,
    initRoomRuin, initRoomTreasure, initRoomHall
  };

  uint8_t roomtbl[MAXRTYPE];
  uint8_t areaidx[MAX_AREA];
  uint8_t roomcnt = m_areacnt / 2; //総エリア数の半分が部屋

  //シャッフルされたindex を取得し、部屋indexとする
  gamemain.getShuffledIdx( areaidx, m_areacnt );


  //各エリアを部屋にする
  for ( uint8_t i = 0; i < roomcnt; i++ ) {
    AREABASE* ab = &m_areabase[ areaidx[i] ];
    uint8_t len = ab->len; //エリアの大きさ
    uint8_t vrcnt = 0;
    //サイズが合っていて使える部屋を集める
    //x!x! フロアのタイプ毎に使える部屋タイプを制限する？
    //x!x! 同じフロアに何個も出ない設定もいる？
    for ( uint8_t j = RTYPE_BARRACKS; j < MAXRTYPE; j++ ) { //Corridor は実際は通路なので含めない
      if ( len >= m_roomdata[j].minsize ) {
        roomtbl[vrcnt++] = j;
      }
    }
    //部屋の種類ランダム化
    gamemain.shuffle( roomtbl, vrcnt );

    //その他条件を考慮しながら使えるものを適用
    //A の部屋がある場合は B の部屋は無いとか。
    ab->rtype = roomtbl[0];
    ab->attr |= AATTR_ROOM; //部屋

    //パラメータに従い設定
    const ROOMDATA* rd = &m_roomdata[ ab->rtype ];
    ab->droplvlcorrection = rd->droplvlcorrection; //ドロップレベル補正
    if ( random(100) < rd->itemrate ) { //アイテムが落ちてる割合
      ab->attr |= AATTR_ITEMDROP;
      ab->numitemdrop = 1 + random( (ab->len + 1) / 2 ); //1:1 2:1 3:1-2 4:1-2 5:1-3 6:1-3
    }
    if ( random(100) < rd->chestrate ) { //宝箱がある割合
      ab->attr |= AATTR_TREASURE;
      ab->numtreasure = 1 + random( (ab->len + 1) / 3 ); //1:1 2:1 3:1 4:1 5:1-2 6:1-2
    }
    if ( random(100) < rd->enemyrate ) { //敵出現
      ab->attr |= AATTR_ENEMY;
      ab->enemynum = rd->minenemy;
      if ( rd->maxenemy > rd->minenemy ) {
        ab->enemynum += random(rd->maxenemy - rd->minenemy);
      }
    }

    //暗闇
    if ( random(100) < 25 ) { //floor で変える？
      ab->attr |= AATTR_DARK;
    }

    //各部屋専用初期化
    if ( !initRoomTbl[ ab->rtype ]( this, ab ) ) {
      //ダメだったら部屋をやめる
      ab->attr &= ~(AATTR_ROOM | AATTR_ITEMDROP | AATTR_TREASURE | AATTR_ENEMY);
    }
  }

  //部屋以外の場所への設定
  for ( int8_t i = 0; i < m_areacnt; i++ ) {
    AREABASE* ab = &m_areabase[ areaidx[i] ];
    if ( ab->attr & AATTR_ROOM ) continue; //部屋は無視

    //暗闇
    if ( random(100) < 30 ) { //floor で変える？
      ab->attr |= AATTR_DARK;
    }

    //敵
    if ( random(100) < 20 ) { //floor で変える？
      ab->attr |= AATTR_ENEMY;
      ab->enemynum += 1 + (ab->len - 1);
      if ( ab->enemynum > 3 ) ab->enemynum = 3;
    }

    //落ちてるアイテム
    if ( random(100) < 10 ) { //floor で変える？
      ab->attr |= AATTR_ITEMDROP;
    }
  }
}

bool CellMaker::initRoomCorridor( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomBarracks( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomPrivateRoom( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomPrison( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomThrone( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomFountain( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomAltar( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomCemetery( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomLaboratory( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomRuin( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomTreasure( CellMaker* cm, AREABASE* ab )
{
  return true;
}

bool CellMaker::initRoomHall( CellMaker* cm, AREABASE* ab )
{
  return true;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CellMaker::setupCell()
{
  for ( int8_t i = 0; i < m_areacnt; i++ ) {
    AREABASE *ab = &m_areabase[i];
    int8_t cx, cy;
    cx = ab->sx;
    cy = ab->sy;

    for ( int8_t j = 0; j < ab->len; j++ ) {
      CELL* c = &m_cell[ cy * TMAPH + cx ];
      c->attr = 0;
      //dark
      if ( ab->attr & AATTR_DARK ) c->attr |= AATTR_DARK; //暗闇はエリア全体に適用。偶に抜く？
#if 0
      //itemdrop
      if ( ab->attr & AATTR_ITEMDROP ) c->attr |= AATTR_ITEMDROP; //エリアの何処に置くか選ぶ。数もチェック。
      //treasure
      if ( ab->attr & AATTR_TREASURE ) c->attr |= AATTR_TREASURE; //エリアの何処に置くか選ぶ。数もチェック。
#endif

      ab->fwdPos( cx, cy );
    }

  }
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------






#if 0
//object 配置test
if ( random(100) < 45 ) {
  if ( random(100) < 75 ) {
    ObjBase* o = createObj( OBJID_TORCH );
    m_blk[i]->setObjWall( o ); //壁に配置
  } else {
    ObjBase* o = createObj( OBJID_CANDLE );
    m_blk[i]->setObjGround( o );  //床に配置
  }
}
#if 01 //container
if ( random(100) < 15 ) {
  if ( random(100) < 30 ) {
    ObjBase* o = createObj( OBJID_CHEST );
#if 01
    setupContainer( static_cast<ObjContainer*>(o), plGetFloor(), 0 ); //中身を入れる
#endif
    m_blk[i]->setObjGround( o );
  } else {
    ObjTable* tbl = static_cast<ObjTable*>( createObj( OBJID_TABLE ) );
    m_blk[i]->setObjGround( tbl );
    //直接作成して机に置く
    ObjCandle* c = static_cast<ObjCandle*>( createObj( OBJID_CANDLE ) );
    if ( c ) {
      tbl->addObj( c ); //ろうそくを机におく
    }
  }
}
#endif
#if 01 //item
if ( random(100) < 7 ) {
  ObjDropItem* o = static_cast<ObjDropItem*>( createObj( OBJID_DROPITEM ) );
  if ( o ) {
    ITEM* item = itGenerateFloor( plGetFloor() );
    o->attachItem( item );
    m_blk[i]->setObjCenter( o ); //通路上に置く
  }
#if 0
  //同じ場所に２個おいて実験
  o = static_cast<ObjDropItem*>( createObj( OBJID_DROPITEM ) );
  if ( o ) {
    ITEM* item = itGenerateFloor( plGetFloor() );
    o->attachItem( item );
    m_blk[i]->setObjCenter( o ); //通路上に置く
  }
#endif
}
#endif
#endif


#if 0 //敵配置 test
if ( m_blkcnt > 1 ) {
  enCreate( ENTYPE( ENTYPE_SLIME + random(ENTYPEMAX - 1) ), 0, id, random(m_blkcnt - 1) + 1 );
}
#endif

