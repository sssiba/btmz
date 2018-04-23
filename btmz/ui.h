#pragma once

#include "btmz.h"


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

class UICtrl
{
public:
  enum : uint8_t {
    BCSLOT_UP,
    BCSLOT_RIGHT,
    BCSLOT_DOWN,
    BCSLOT_LEFT,
    BCSLOTMAX
  };
  enum : uint8_t {
    BCMD_EMPTY,
    BCMD_A, //icon 用 dummy
    BCMD_B, //icon 用 dummy
    BCMD_CROSS,  //icon 用 dummy
    BCMD_ATTACK,
    BCMD_GET,
    BCMD_DOWN,
    BCMD_UP,
  };

public:
  UICtrl();
  ~UICtrl();

  void update();
  void draw();

  void openBtnCmd( int btn, uint8_t slotup, uint8_t slotright, uint8_t slotdown, uint8_t slotleft );
  void closeBtnCmd();

private:
  int8_t m_btncmd; //-1:nothing  0:btnA  1:btnB
  uint8_t m_btncmdslot[BCSLOTMAX];
};

extern UICtrl g_uictrl;

#define UIC() g_uictrl

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
