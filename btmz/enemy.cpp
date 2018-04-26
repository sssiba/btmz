#include "btmz.h"


#include "player.h"

#include "enemy.h"

#include "dungeon.h"

#define ENTPL( _ID_ ) (&g_enTemplate[ _ID_ ])

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static void enDead( EnemyData* ed );

static void enSlimeInit(EnemyData*);
static void enSlimeUpdate(EnemyData*);
static void enSlimeDraw(EnemyData*);
static void enSlimeFinish(EnemyData*);
/*
 * 敵データテンプレート
 */
EnemyTemplate g_enTemplate[ ENTYPEMAX ] = {
  { //dummy
  },
  { //slime
//    &epSlime, //pic
    enSlimeInit,
    enSlimeUpdate,
    enSlimeDraw,
    enSlimeFinish,
    "Slime", //name
    1, //lvl
    15, //hpmax
    5, //sr
    3, //def
    { -8, -8, 16, 8 }, //移動当たり判定
    {  8, -6,  4, 3 }, //攻撃当たり判定
    { -8, -8, 16, 8 }, //ダメージ当たり判定
  },  
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static EnemyData g_endata[MAX_ENEMYENTRY];


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void enInit()
{
  memset( g_endata, 0, sizeof(g_endata) );
}

void enUpdate()
{
  for( int8_t i=0; i<MAX_ENEMYENTRY; i++ ) {
    if( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];
    if( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    ENTPL( ed->type )->fUpdate( ed );
  }
}

void enDraw()
{
  for( int8_t i=0; i<MAX_ENEMYENTRY; i++ ) {
    if( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];

    if( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    ENTPL( ed->type )->fDraw( ed );    
    
#if defined( DBG_SHOW_HITRECT )
  {
    EnemyTemplate* et = ENTPL( ed->type );
    int16_t x, y, rx, ry;
    x = DUNMAP()->toScrX(TOINT(ed->x));
    y = DUNMAP()->toScrY(TOINT(ed->y));
    //move(green)
    if( ed->flip ) {
      rx = x - et->mvrect.x - et->mvrect.w;
    } else {
      rx = x + et->mvrect.x;
    }
    ry = y + et->mvrect.y;
    gb.display.setColor( Color::lightgreen );
    gb.display.drawRect( rx, ry, et->mvrect.w, et->mvrect.h );
    //attack(red)
    if( ed->flip ) {
      rx = x - et->atrect.x - et->atrect.w;
    } else {
      rx = x + et->atrect.x;
    }
    ry = y + et->atrect.y;
    gb.display.setColor( Color::red );
    gb.display.drawRect( rx, ry, et->atrect.w, et->atrect.h );
    //defense(blue)
    if( ed->flip ) {
      rx = x - et->dfrect.x - et->dfrect.w;
    } else {
      rx = x + et->dfrect.x;
    }
    ry = y + et->dfrect.y;
    gb.display.setColor( Color::lightblue );
    gb.display.drawRect( rx, ry, et->dfrect.w, et->dfrect.h );
  }
#endif

   }
}

void enFinish()
{
  
}

int8_t enCreate( ENTYPE ent, uint8_t scl, uint8_t area, uint8_t blk )
{
  //empty
  int8_t i=0;
  for( i=0; i<MAX_ENEMYENTRY; i++ ) {
    if( g_endata[i].type == ENTYPE_UNDEFINED ) break;
  }
  if( i == MAX_ENEMYENTRY ) return -1;

  EnemyData* ed = &g_endata[i];
  ed->type = ent;

  EnemyTemplate* et = ENTPL( ed->type );
  
  ed->lvl = et->lvl;
  ed->hp = ed->hpmax = et->hpmax;
  ed->str = et->str;
  ed->def = et->def;
  ed->stat = 0;
  ed->area = area;
  ed->block = blk;
  ed->x = TOFIX( blk * BLKTILEW * TILEW + (BLKTILEW * TILEW / 2) );
  ed->y = TOFIX( (BLKTILEH - 2) * TILEH + (TILEH / 2) );
  ed->flip = true;
  ed->anm = 0;
  ed->anmwait = 0;
  ed->phase = ENPHASE_WAIT;

  et->fInit( ed );
  
  return i;
}

/*
 * 敵とプレイヤーとの x 軸上の距離を取得
 */
int16_t enGetPlayerDist( EnemyData* ed )
{
  int16_t ret = plGetFixX() - ed->x;
  return (ret < 0) ? -ret : ret;
}

/*
 * 各当たり判定領域との当たり判定
 */
enum : int8_t {
  RTYPE_MV,
  RTYPE_AT,
  RTYPE_DF,
};

static EnemyData* enCheckRect( int8_t type, int16_t x, int16_t y, Rect8& rect, bool flip )
{
  int16_t sx0, sx1;
  if( flip ) {
    sx0 = x - rect.x - rect.w;
  } else {
    sx0 = x + rect.x;
  }
  sx1 = sx0 + rect.w;
  int16_t sy0 = y + rect.y;
  int16_t sy1 = sy0 + rect.h;
  
  for( int8_t i=0; i<MAX_ENEMYENTRY; i++ ) {
    if( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];
    if( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    EnemyTemplate* et = ENTPL( ed->type );

    int16_t dx0 = TOINT(ed->x);// + et->mvrect.x;
    int16_t dy0 = TOINT(ed->y);// + et->mvrect.y;
    int16_t dx1, dy1;

    int16_t rx, rw, ry, rh;
    Rect8* trect;
    switch( type ) {
      case RTYPE_MV: trect = &et->mvrect; break;
      case RTYPE_AT: trect = &et->atrect; break;
      case RTYPE_DF: trect = &et->dfrect; break;
    }

    if( ed->flip ) {
      dx0 -= trect->x;
      dx1 = dx0 - trect->w;
    } else {
      dx0 += trect->x;
      dx1 = dx0 + trect->w;
    }
    if( dx1 < dx0 ) {
      int16_t t = dx0;
      dx0 = dx1;
      dx1 = t;
    }
    
    dy0 += trect->y;
    dy1 = dy0 + trect->h;

    

    if(sx0 <= dx1 && dx0 <= sx1 && sy0 <= dy1 && dy0 <= sy1) {
      return ed;    
    }
  }

  return NULL;
}


EnemyData* enCheckMvRect( int16_t x, int16_t y, Rect8& rect, bool flip )
{
  return enCheckRect( RTYPE_MV, x, y, rect, flip );
}

EnemyData* enCheckDfRect( int16_t x, int16_t y, Rect8& rect, bool flip )
{
  return enCheckRect( RTYPE_DF, x, y, rect, flip );
}

/*
 * プレイヤーの攻撃で受けるダメージを計算する
 *  ed 対象となる敵。NULL の場合は純粋にプレイヤーの攻撃力のみ返す
 */
int16_t enCalcDamage( EnemyData* ed )
{
  /*
   * damage:
   * patk = (pamin+pamax)/2 + str*0.5 + (str*0.5 * lvl)/100
   * 
   * (patk * patk) / (patk + edef)
   */
  PLSTAT& ps = plGetStat();
  float dmg;
  float patk = (ps.minatk + ps.maxatk)/2.0f + ps.str*0.5f + (ps.str*0.5f * ps.lvl) / 100;
  if( ed ) {
    dmg = (patk * patk) / (patk + ed->def);
  } else {
    dmg = patk;
  }

  return (int16_t)dmg;
}


/*
 * 指定の値のダメージを受ける
 * ダメージ値自体は別に計算。ここでは必ず指定の値ダメージを受ける。
 * 
 * return true:死んだ  false:生きてる
 */
bool enDamage( EnemyData* ed, int16_t dmg )
{
  //x!x! 高さは取り敢えずプレイヤーの武器っぽい、足元から -12 位にしておく
  //x!x! x も当たり判定辺りにしてしまう？
  UIC().dispNumUp( ColorIndex::red, dmg, TOINT(ed->x), TOINT(ed->y)-12  ); 
  
  ed->hp -= dmg;
  if( ed->hp <= 0 ) {
    //dead
    enDead( ed );
    return true;
  }

  return false;
}

void enDead( EnemyData* ed )
{
  ed->type = ENTYPE_UNDEFINED;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void enSlimeInit( EnemyData* ed )
{
}

void enSlimeUpdate( EnemyData* ed )
{
  int16_t mv = 0;
  int16_t cx, cy;

  if( ed->phase == ENPHASE_COOLDOWN ) {
    //攻撃後待機中
    if( ed->phasewait == 0 ) {
      ed->phase = ENPHASE_MOVE;
    } else {
      ed->phasewait--;
    }
  } else
  if( ed->phase == ENPHASE_ATTACK ) {
    ed->phase = ENPHASE_COOLDOWN;
    ed->phasewait = 13;
  } else {
    if( enGetPlayerDist( ed ) < TOFIX(50) ) {
      if( ed->x < plGetFixX() ) {
        mv = TOFIX( 0.1f );
        ed->flip = false;
      } else
      if( ed->x > plGetFixX() ) {
        mv = TOFIX( -0.1f );
        ed->flip = true;
      }
    }

    cx = ed->x;
    cy = ed->y;
    cx += ed->flip ? TOFIX(-8) : TOFIX(8);
  
    cx += mv;
    bool movable = true;
    uint8_t bg = 0;
    bg = DUNMAP()->getMapBG( TOINT(cx), TOINT(cy) );
    movable = (bg == 0 );

    EnemyTemplate* et = ENTPL( ed->type );
    { //移動当たり判定
      int16_t tx = TOINT(ed->x + mv);
      int16_t ty = TOINT(ed->y);
      if( plCheckMvRect( tx, ty, et->mvrect, ed->flip ) ) {
        movable = false;
      }

      //x!x! 他の敵ともやる？
    }

    //攻撃する？
    {
      if( plCheckDfRect( TOINT(ed->x), TOINT(ed->y), et->atrect, ed->flip ) ) {
        //自分の攻撃範囲にプレイヤーがいたら攻撃
        ed->phase = ENPHASE_ATTACK;

        int16_t dmg = plCalcDamage( ed );
        plDamage( ed, dmg );

        //攻撃の際は移動しない
        movable = false;
      }
    }
  
    if( movable ) {
      ed->x += mv; 
      ed->phase = ENPHASE_MOVE;
    }
  }
}

void enSlimeDraw( EnemyData* ed )
{
    EnemyPic* ep = &epSlime;//ENTPL( ed->type )->pic;

    if( ed->phase == ENPHASE_ATTACK ||
        ed->phase == ENPHASE_COOLDOWN ) {
      ep->img->setFrame( 2 );
      gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x))-(ep->w / 2),
                            DUNMAP()->toScrY(TOINT(ed->y))-(ep->h),
                            *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
    } else {
      ep->img->setFrame( ep->move[ed->anm] );
      gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x))-(ep->w / 2),
                            DUNMAP()->toScrY(TOINT(ed->y))-(ep->h),
                            *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
      if( ed->anmwait == 0 ) {
        ed->anm ^= 1;
        ed->anmwait = 5;
      } else {
        ed->anmwait--;
      }
    }
}

void enSlimeFinish( EnemyData* ed )
{
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

