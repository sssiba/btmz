#include "ui.h"

#include "pic.h"

#include "dungeon.h"


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
UICtrl g_uictrl;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
UICtrl::UICtrl()
  : m_btncmd(BCMD_EMPTY)
{
  for( int8_t i=0; i<BCSLOTMAX; i++ ) m_btncmdslot[i] = BCMD_EMPTY;
  memset( m_dispnum, 0, sizeof(m_dispnum) );
}

UICtrl::~UICtrl()
{
  
}

void UICtrl::update()
{
  //disp num
//  updateDispNum();
}


void UICtrl::draw()
{
  //A/B command
  drawBtnCmd();

  //dispnum
  drawDispNum();
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void UICtrl::entryDispNum( ColorIndex c, int16_t v, int8_t x, int8_t y, int8_t my )
{
  int8_t i;
  if( v > 30000 ) v = 30000;
  for( i=0; i<MAX_DISPNUM; i++ ) {
    if( !m_dispnum[i].duration ) {
      m_dispnum[i].duration = 3;
      m_dispnum[i].my = my;
      m_dispnum[i].c = c;
      sprintf( m_dispnum[i].v, "%d", v );

      int8_t d = strlen(m_dispnum[i].v);
      d *= FONTW;

      m_dispnum[i].x = DUNMAP()->toScrX(x) - (d>>1);
      m_dispnum[i].y = DUNMAP()->toScrY(y);
    
    }
  }
}

void UICtrl::drawDispNum()
{
  for( int8_t i=0; i<MAX_DISPNUM; i++ ) {
    DISPNUM* dn = &m_dispnum[i];
    if( dn->duration ) {
      dn->y += dn->my;

      gb.display.setColor( dn->c );
      gb.display.setCursor( dn->x, dn->y );
      gb.display.println( dn->v );
      dn->duration--;
    }
  }
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void UICtrl::openBtnCmd( int btn, uint8_t slotup, uint8_t slotright, uint8_t slotdown, uint8_t slotleft )
{
  m_btncmd = btn;
  m_btncmdslot[BCSLOT_UP] = slotup;   
  m_btncmdslot[BCSLOT_RIGHT] = slotright;
  m_btncmdslot[BCSLOT_DOWN] = slotdown;
  m_btncmdslot[BCSLOT_LEFT] = slotleft;
}
void UICtrl::closeBtnCmd()
{
  m_btncmd = BCMD_EMPTY;
}

void UICtrl::drawBtnCmd()
{
  int8_t ofstx[BCSLOTMAX] = { 0, 5, 0, -5 };
  int8_t ofsty[BCSLOTMAX] = { -5, 0, 5, 0 };

  static const int8_t AX = 60;
  static const int8_t AY = 5;
  static const int8_t BX = 70;
  static const int8_t BY = 5;

  static const int8_t bpos[] = {
    60, 5, //A
    70, 5, //B
  };
  
  if( m_btncmd == BCMD_EMPTY ) {
    //どちらも押されてない
    getPic( PIC_ICON6x5 )->setFrame( ICON_A ); //A
    gb.display.drawImage( bpos[0*2+0], bpos[0*2+1], *getPic( PIC_ICON6x5 ) );
    getPic( PIC_ICON6x5 )->setFrame( ICON_B ); //B
    gb.display.drawImage( bpos[1*2+0], bpos[1*2+1], *getPic( PIC_ICON6x5 ) );
  } else {
    //A,B どちから起動中
    int8_t b = m_btncmd - BCMD_A;
    int8_t ob = b ^ 1;
    int8_t x = bpos[b*2+0];
    int8_t y = bpos[b*2+1];
    getPic( PIC_ICON6x5 )->setFrame( ICON_CROSS ); //+
    gb.display.drawImage( x, y, *getPic( PIC_ICON6x5 ) );
    for( int8_t i=0; i<BCSLOTMAX; i++ ) {
      getPic( PIC_ICON6x5 )->setFrame( m_btncmdslot[i] );
      gb.display.drawImage( x+ofstx[i], y+ofsty[i], *getPic( PIC_ICON6x5 ) );
    }
    getPic( PIC_ICON6x5 )->setFrame( ICON_A+ob ); //other button
    gb.display.drawImage( bpos[ob*2+0], bpos[ob*2+1], *getPic( PIC_ICON6x5 ) );
  }
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


