#include "btmz.h"


#include "player.h"

#include "enemy.h"

#include "dungeon.h"


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
static void enDead( EnemyData* ed );

static void enSlimeInit(EnemyData*);
static void enSlimeUpdate(EnemyData*);
static void enSlimeDraw(EnemyData*);
static void enSlimeFinish(EnemyData*);

static void enGhostInit(EnemyData*);
static void enGhostUpdate(EnemyData*);
static void enGhostDraw(EnemyData*);
static void enGhostFinish(EnemyData*);

static void enOrcInit(EnemyData*);
static void enOrcUpdate(EnemyData*);
static void enOrcDraw(EnemyData*);
static void enOrcFinish(EnemyData*);

/*
   敵データテンプレート
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
    30, //atkspd
    16, //w
    2, //getexp
    { -8, -8, 16, 8 }, //移動当たり判定
    {  8, -6,  4, 3 }, //攻撃当たり判定
    { -8, -8, 16, 8 }, //ダメージ当たり判定
  },
  { //ghost
    enGhostInit,
    enGhostUpdate,
    enGhostDraw,
    enGhostFinish,
    "Ghost", //name
    2, //lvl
    20, //hpmax
    3, //str
    5, //def
    25, //atkspd
    6, //w
    4, //getexp
    { -3, -16, 6, 16 }, //移動当たり判定
    {  3, -10, 5, 4 },  //攻撃当たり判定
    { -4, -16, 8, 16 }, //ダメージ当たり判定
  },
  { //orc
    enOrcInit,
    enOrcUpdate,
    enOrcDraw,
    enOrcFinish,
    "Orc", //name
    3, //lvl
    25, //hpmax
    7, //str
    7, //def
    30, //atkspd
    12, //w
    8, //getexp
    { -6, -16, 12, 16 }, //移動当たり判定
    {  6, -9,  6, 4 }, //攻撃当たり判定
    { -6, -16, 12, 16 }, //ダメージ当たり判定
  }
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
  for ( int8_t i = 0; i < MAX_ENEMYENTRY; i++ ) {
    if ( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];
    if ( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    //攻撃間隔減らす
    if ( ed->atkinterval ) ed->atkinterval--;

    ENTPL( ed->type )->fUpdate( ed );
  }
}

void enDraw()
{
  for ( int8_t i = 0; i < MAX_ENEMYENTRY; i++ ) {
    if ( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];

    if ( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    ENTPL( ed->type )->fDraw( ed );

#if defined( DBG_SHOW_HITRECT )
    {
      EnemyTemplate* et = ENTPL( ed->type );
      int16_t x, y, rx, ry;
      x = DUNMAP()->toScrX(TOINT(ed->x));
      y = DUNMAP()->toScrY(TOINT(ed->y));
      //move(green)
      if ( ed->flip ) {
        rx = x - et->mvrect.x - et->mvrect.w;
      } else {
        rx = x + et->mvrect.x;
      }
      ry = y + et->mvrect.y;
      gb.display.setColor( Color::lightgreen );
      gb.display.drawRect( rx, ry, et->mvrect.w, et->mvrect.h );
      //attack(red)
      if ( ed->flip ) {
        rx = x - et->atrect.x - et->atrect.w;
      } else {
        rx = x + et->atrect.x;
      }
      ry = y + et->atrect.y;
      gb.display.setColor( Color::red );
      gb.display.drawRect( rx, ry, et->atrect.w, et->atrect.h );
      //defense(blue)
      if ( ed->flip ) {
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

EnemyData* enCreate( ENTYPE ent, uint8_t scl, uint8_t area )
{
  //empty
  int8_t i = 0;
  for ( i = 0; i < MAX_ENEMYENTRY; i++ ) {
    if ( g_endata[i].type == ENTYPE_UNDEFINED ) break;
  }
  if ( i == MAX_ENEMYENTRY ) return NULL;

  EnemyData* ed = &g_endata[i];
  ed->type = ent;

  EnemyTemplate* et = ENTPL( ed->type );

  ed->lvl = et->lvl;
  ed->hp = ed->hpmax = et->hpmax;
  ed->str = et->str;
  ed->def = et->def;
  ed->atkinterval = 0;
  ed->stat = 0;
  ed->area = area;
  //  ed->block = blk;
  //  ed->x = x;//TOFIX( blk * BLKTILEW * TILEW + (BLKTILEW * TILEW / 2) );
  //  ed->y = y;//TOFIX( (BLKTILEH - 2) * TILEH + (TILEH / 2) );
  ed->flip = true;
  ed->anm = 0;
  ed->anmwait = 0;
  ed->phase = ENPHASE_WAIT;

  et->fInit( ed );

  return ed;
}


/*
   敵とプレイヤーとの x 軸上の距離を取得
*/
int16_t enGetPlayerDist( EnemyData* ed )
{
  int16_t ret = plGetFixX() - ed->x;
  return (ret < 0) ? -ret : ret;
}

/*
   各当たり判定領域との当たり判定
*/
enum : int8_t {
  RTYPE_MV,
  RTYPE_AT,
  RTYPE_DF,
};

static EnemyData* enCheckRect( int8_t type, int16_t x, int16_t y, Rect8& rect, bool flip )
{
  int16_t sx0, sx1;
  if ( flip ) {
    sx0 = x - rect.x - rect.w;
  } else {
    sx0 = x + rect.x;
  }
  sx1 = sx0 + rect.w;
  int16_t sy0 = y + rect.y;
  int16_t sy1 = sy0 + rect.h;

  for ( int8_t i = 0; i < MAX_ENEMYENTRY; i++ ) {
    if ( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];
    if ( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    EnemyTemplate* et = ENTPL( ed->type );

    int16_t dx0 = TOINT(ed->x);// + et->mvrect.x;
    int16_t dy0 = TOINT(ed->y);// + et->mvrect.y;
    int16_t dx1, dy1;

    int16_t rx, rw, ry, rh;
    Rect8* trect;
    switch ( type ) {
      case RTYPE_MV: trect = &et->mvrect; break;
      case RTYPE_AT: trect = &et->atrect; break;
      case RTYPE_DF: trect = &et->dfrect; break;
    }

    if ( ed->flip ) {
      dx0 -= trect->x;
      dx1 = dx0 - trect->w;
    } else {
      dx0 += trect->x;
      dx1 = dx0 + trect->w;
    }
    if ( dx1 < dx0 ) {
      int16_t t = dx0;
      dx0 = dx1;
      dx1 = t;
    }

    dy0 += trect->y;
    dy1 = dy0 + trect->h;



    if (sx0 <= dx1 && dx0 <= sx1 && sy0 <= dy1 && dy0 <= sy1) {
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

EnemyData* enGetInRange( int16_t x, int16_t y, int16_t range )
{
  int16_t sx0, sx1;

  sx0 = x - range;
  sx1 = x + range;

  for ( int8_t i = 0; i < MAX_ENEMYENTRY; i++ ) {
    if ( g_endata[i].type == ENTYPE_UNDEFINED ) continue;
    EnemyData* ed = &g_endata[i];
    if ( ed->area != DUNMAP()->getCurAreaIdx() ) continue;

    EnemyTemplate* et = ENTPL( ed->type );

    Rect8* trect = &et->dfrect;
    int16_t dx0 = TOINT(ed->x), dx1;
    if ( ed->flip ) {
      dx0 -= trect->x;
      dx1 = dx0 - trect->w;
    } else {
      dx0 += trect->x;
      dx1 = dx0 + trect->w;
    }

    if (sx0 <= dx1 && dx0 <= sx1 ) {
      return ed;
    }
  }

  return NULL;
}

/*
   プレイヤーの攻撃で受けるダメージを計算する
    ed 対象となる敵。NULL の場合は純粋にプレイヤーの攻撃力のみ返す
*/
int16_t enCalcDamage( EnemyData* ed )
{
  /*
     damage:
     patk = (pamin+pamax)/2 + str*0.5 + (str*0.5 * lvl)/100

     (patk * patk) / (patk + edef)
  */
  //  PLSTAT& ps = plGetStat();
  float dmg;
  float patk = (plGetMINATK() + plGetMAXATK()) / 2.0f + plGetSTR() * 0.5f + (plGetSTR() * 0.5f * plGetLVL()) / 100;
  patk *= 1.0f + (plGetADDDMG() / 100.0f);
  if ( ed ) {
    dmg = (patk * patk) / (patk + ed->def);
  } else {
    dmg = patk;
  }

  return (int16_t)dmg;
}


/*
   指定の値のダメージを受ける
   ダメージ値自体は別に計算。ここでは必ず指定の値ダメージを受ける。

   return true:死んだ  false:生きてる
*/
bool enDamage( EnemyData* ed, int16_t dmg )
{
  //x!x! 高さは取り敢えずプレイヤーの武器っぽい、足元から -12 位にしておく
  //x!x! x も当たり判定辺りにしてしまう？
  UIC().dispNumUp( ColorIndex::red, dmg, TOINT(ed->x), TOINT(ed->y) - 12  );

  ed->hp -= dmg;
  if ( ed->hp <= 0 ) {
    //dead
    enDead( ed );
    return true;
  }

  return false;
}

void enDead( EnemyData* ed )
{
  EnemyTemplate* et = ENTPL( ed->type );
  char s[22];
  sprintf( s, "Killed %s", et->name );
  showModelessInfo (s, 20 );
  ed->type = ENTYPE_UNDEFINED;

  plAddExp( et->getexp );

}

void enSave( File& f )
{
  f.write( &g_endata, sizeof(g_endata) );
}

bool enLoad( File& f )
{
#if defined( DBG_SAVELOAD )
  {
    TRACE( "LOAD>endata" );
  }
#endif

  f.read( &g_endata, sizeof(g_endata) );

  return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline bool enIsAttackable( EnemyData* ed )
{
  return (ed->atkinterval == 0);
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void enSlimeInit( EnemyData* ed )
{
}

void enSlimeUpdate( EnemyData* ed )
{
  EnemyPic* ep = &epSlime;//ENTPL( ed->type )->pic;
  int16_t mv = 0;
  int16_t cx, cy;

  if ( ed->phase == ENPHASE_COOLDOWN ) {
    //攻撃後待機中
    if ( ed->phasewait == 0 ) {
      ed->phase = ENPHASE_MOVE;
    } else {
      ed->phasewait--;
    }
  } else if ( ed->phase == ENPHASE_ATTACK ) {
    ed->phase = ENPHASE_COOLDOWN;
    ed->phasewait = 13;
  } else {
    if ( enGetPlayerDist( ed ) < TOFIX(50) ) {
      if ( ed->x < plGetFixX() ) {
        mv = TOFIX( 0.1f );
        ed->flip = false;
      } else if ( ed->x > plGetFixX() ) {
        mv = TOFIX( -0.1f );
        ed->flip = true;
      }
    }

    cx = ed->x;
    cy = ed->y;
    cx += ed->flip ? TOFIX(-ep->w / 2) : TOFIX(ep->w / 2);

    cx += mv;
    bool movable = true;
    uint8_t bgattr = DUNMAP()->getAttrBG( TOINT(cx), TOINT(cy) );
    movable = !BGisBlock(bgattr);

    EnemyTemplate* et = ENTPL( ed->type );
    { //移動当たり判定
      int16_t tx = TOINT(ed->x + mv);
      int16_t ty = TOINT(ed->y);
      if ( plCheckMvRect( tx, ty, et->mvrect, ed->flip ) ) {
        movable = false;
      }

      //x!x! 他の敵ともやる？
    }

    //攻撃する？
    {
      if ( enIsAttackable(ed) ) {
        if ( plCheckDfRect( TOINT(ed->x), TOINT(ed->y), et->atrect, ed->flip ) ) {
          //自分の攻撃範囲にプレイヤーがいたら攻撃
          ed->phase = ENPHASE_ATTACK;

          //攻撃間隔リセット
          ed->atkinterval = et->atkspd;

          int16_t dmg = plCalcDamage( ed );
          plDamage( dmg );

          //攻撃の際は移動しない
          movable = false;
        }
      }
    }

    if ( movable ) {
      ed->x += mv;
      ed->phase = ENPHASE_MOVE;
    }
  }
}

void enSlimeDraw( EnemyData* ed )
{
  EnemyPic* ep = &epSlime;//ENTPL( ed->type )->pic;

  if ( ed->phase == ENPHASE_ATTACK ||
       ed->phase == ENPHASE_COOLDOWN ) {
    ep->img->setFrame( ep->attack );
    gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x)) - (ep->w / 2),
                          DUNMAP()->toScrY(TOINT(ed->y)) - (ep->h),
                          *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
  } else {
    ep->img->setFrame( ep->move[ed->anm] );
    gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x)) - (ep->w / 2),
                          DUNMAP()->toScrY(TOINT(ed->y)) - (ep->h),
                          *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
    if ( ed->anmwait == 0 ) {
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
void enGhostInit( EnemyData* ed )
{
}

void enGhostUpdate( EnemyData* ed )
{
  EnemyPic* ep = &epGhost;//ENTPL( ed->type )->pic;
  int16_t mv = 0;
  int16_t cx, cy;

  if ( ed->phase == ENPHASE_COOLDOWN ) {
    //攻撃後待機中
    if ( ed->phasewait == 0 ) {
      ed->phase = ENPHASE_MOVE;
    } else {
      ed->phasewait--;
    }
  } else if ( ed->phase == ENPHASE_ATTACK ) {
    ed->phase = ENPHASE_COOLDOWN;
    ed->phasewait = 13;
  } else {
    if ( enGetPlayerDist( ed ) < TOFIX(30) ) {
      if ( ed->x < plGetFixX() ) {
        mv = TOFIX( 0.5f );
        ed->flip = false;
      } else if ( ed->x > plGetFixX() ) {
        mv = TOFIX( -0.5f );
        ed->flip = true;
      }
    }

    cx = ed->x;
    cy = ed->y;
    cx += ed->flip ? TOFIX(-ep->w / 2) : TOFIX(ep->w / 2);

    cx += mv;
    bool movable = true;
    uint8_t bgattr = DUNMAP()->getAttrBG( TOINT(cx), TOINT(cy) );
    movable = !BGisBlock(bgattr);

    EnemyTemplate* et = ENTPL( ed->type );
    { //移動当たり判定
      int16_t tx = TOINT(ed->x + mv);
      int16_t ty = TOINT(ed->y);
      if ( plCheckMvRect( tx, ty, et->mvrect, ed->flip ) ) {
        movable = false;
      }

      //x!x! 他の敵ともやる？
    }

    //攻撃する？
    {
      if ( enIsAttackable(ed) ) {
        if ( plCheckDfRect( TOINT(ed->x), TOINT(ed->y), et->atrect, ed->flip ) ) {
          //自分の攻撃範囲にプレイヤーがいたら攻撃
          ed->phase = ENPHASE_ATTACK;

          //攻撃間隔リセット
          ed->atkinterval = et->atkspd;

          int16_t dmg = plCalcDamage( ed );
          plDamage( dmg );

          //攻撃の際は移動しない
          movable = false;
        }
      }
    }

    if ( movable ) {
      ed->x += mv;
      ed->phase = ENPHASE_MOVE;
    }
  }
}

void enGhostDraw( EnemyData* ed )
{
  EnemyPic* ep = &epGhost;//ENTPL( ed->type )->pic;

  if ( ed->phase == ENPHASE_ATTACK ||
       ed->phase == ENPHASE_COOLDOWN ) {
    ep->img->setFrame( ep->attack );
    gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x)) - (ep->w / 2),
                          DUNMAP()->toScrY(TOINT(ed->y)) - (ep->h),
                          *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
  } else {
    ep->img->setFrame( ep->move[ed->anm] );
    gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x)) - (ep->w / 2),
                          DUNMAP()->toScrY(TOINT(ed->y)) - (ep->h),
                          *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
    if ( ed->anmwait == 0 ) {
      ed->anm ^= 1;
      ed->anmwait = 5;
    } else {
      ed->anmwait--;
    }
  }
}

void enGhostFinish( EnemyData* ed )
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void enOrcInit( EnemyData* ed )
{

}

void enOrcUpdate( EnemyData* ed )
{
  EnemyPic* ep = &epOrc;//ENTPL( ed->type )->pic;
  int16_t mv = 0;
  int16_t cx, cy;

  if ( ed->phase == ENPHASE_COOLDOWN ) {
    //攻撃後待機中
    if ( ed->phasewait == 0 ) {
      ed->phase = ENPHASE_MOVE;
    } else {
      ed->phasewait--;
    }
  } else if ( ed->phase == ENPHASE_ATTACK ) {
    ed->phase = ENPHASE_COOLDOWN;
    ed->phasewait = 13;
  } else {
    if ( enGetPlayerDist( ed ) < TOFIX(30) ) {
      if ( ed->x < plGetFixX() ) {
        mv = TOFIX( 0.5f );
        ed->flip = false;
      } else if ( ed->x > plGetFixX() ) {
        mv = TOFIX( -0.5f );
        ed->flip = true;
      }
    }

    cx = ed->x;
    cy = ed->y;
    cx += ed->flip ? TOFIX(-ep->w / 2) : TOFIX(ep->w / 2);

    cx += mv;
    bool movable = true;
    uint8_t bgattr = DUNMAP()->getAttrBG( TOINT(cx), TOINT(cy) );
    movable = !BGisBlock(bgattr);

    EnemyTemplate* et = ENTPL( ed->type );
    { //移動当たり判定
      int16_t tx = TOINT(ed->x + mv);
      int16_t ty = TOINT(ed->y);
      if ( plCheckMvRect( tx, ty, et->mvrect, ed->flip ) ) {
        movable = false;
      }

      //x!x! 他の敵ともやる？
    }

    //攻撃する？
    {
      if ( enIsAttackable(ed) ) {
        if ( plCheckDfRect( TOINT(ed->x), TOINT(ed->y), et->atrect, ed->flip ) ) {
          //自分の攻撃範囲にプレイヤーがいたら攻撃
          ed->phase = ENPHASE_ATTACK;

          //攻撃間隔リセット
          ed->atkinterval = et->atkspd;
          
          int16_t dmg = plCalcDamage( ed );
          plDamage( dmg );

          //攻撃の際は移動しない
          movable = false;
        }
      }
    }

    if ( movable ) {
      ed->x += mv;
      ed->phase = ENPHASE_MOVE;
    }
  }
}

void enOrcDraw( EnemyData* ed )
{
  EnemyPic* ep = &epOrc;//ENTPL( ed->type )->pic;

  if ( ed->phase == ENPHASE_ATTACK ||
       ed->phase == ENPHASE_COOLDOWN ) {
    ep->img->setFrame( ep->attack );
    gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x)) - (ep->w / 2),
                          DUNMAP()->toScrY(TOINT(ed->y)) - (ep->h),
                          *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
  } else {
    ep->img->setFrame( ep->move[ed->anm] );
    gb.display.drawImage( DUNMAP()->toScrX(TOINT(ed->x)) - (ep->w / 2),
                          DUNMAP()->toScrY(TOINT(ed->y)) - (ep->h),
                          *ep->img, ed->flip ? -ep->w : ep->w, ep->h );
    if ( ed->anmwait == 0 ) {
      ed->anm ^= 1;
      ed->anmwait = 5;
    } else {
      ed->anmwait--;
    }
  }
}

void enOrcFinish( EnemyData* ed )
{

}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

