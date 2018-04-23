#include "ui.h"

#include "pic.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
UICtrl g_uictrl;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
UICtrl::UICtrl()
  : m_btncmd(-1)
{
  for( int8_t i=0; i<BCSLOTMAX; i++ ) m_btncmdslot[i] = BCMD_EMPTY;
}

UICtrl::~UICtrl()
{
  
}

void UICtrl::update()
{
}


void UICtrl::draw()
{
  //A/B command
  int8_t ofstx[BCSLOTMAX] = { 0, 5, 0, -5 };
  int8_t ofsty[BCSLOTMAX] = { -5, 0, 5, 0 };

  static const int8_t AX = 60;
  static const int8_t AY = 5;
  static const int8_t BX = 70;
  static const int8_t BY = 5;
  
  switch( m_btncmd ) {
    case 0: //A
      getPic( PIC_ICON6x5 )->setFrame( 3 ); //+
      gb.display.drawImage( AX, AY, *getPic( PIC_ICON6x5 ) );
      for( int8_t i=0; i<BCSLOTMAX; i++ ) {
        getPic( PIC_ICON6x5 )->setFrame( m_btncmdslot[i] );
        gb.display.drawImage( AX+ofstx[i], AY+ofsty[i], *getPic( PIC_ICON6x5 ) );
      }
      getPic( PIC_ICON6x5 )->setFrame( 2 ); //B
      gb.display.drawImage( BX, BY, *getPic( PIC_ICON6x5 ) );
      break;
    case 1: //B
      getPic( PIC_ICON6x5 )->setFrame( 3 ); //+
      gb.display.drawImage( BX, BY, *getPic( PIC_ICON6x5 ) );
      for( int8_t i=0; i<BCSLOTMAX; i++ ) {
        getPic( PIC_ICON6x5 )->setFrame( m_btncmdslot[i] );
        gb.display.drawImage( BX+ofstx[i], BY+ofsty[i], *getPic( PIC_ICON6x5 ) );
      }
      getPic( PIC_ICON6x5 )->setFrame( 1 ); //A
      gb.display.drawImage( AX, AY, *getPic( PIC_ICON6x5 ) );
      break;
    default:
      getPic( PIC_ICON6x5 )->setFrame( 1 ); //A
      gb.display.drawImage( AX, AY, *getPic( PIC_ICON6x5 ) );
      getPic( PIC_ICON6x5 )->setFrame( 2 ); //B
      gb.display.drawImage( BX, BY, *getPic( PIC_ICON6x5 ) );
      break;
  }
}

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
  m_btncmd = -1;
}




//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


