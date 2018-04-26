#pragma once

#include "btmz.h"


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

class UICtrl
{
public:
  enum : uint8_t {
    ICON_CMDEMPTY,
    ICON_A,  //A button
    ICON_B,  //B button
    ICON_CROSS, //Dpad
    ICON_ATTACK, //attack
    ICON_GET,    //get
    ICON_DOWN,   //down
    ICON_UP,     //up
    ICON_LOOT,   //loot(open) container
    ICON_DEFENSE, //defense
    ICON_HEART,   //HP
  };
  enum : uint8_t {
    BCSLOT_UP,
    BCSLOT_RIGHT,
    BCSLOT_DOWN,
    BCSLOT_LEFT,
    BCSLOTMAX
  };
  enum : uint8_t { //ボタン用コマンド。アイコンの並びと対応させたい。
    BCMD_EMPTY,
    BCMD_A, //icon 用 dummy
    BCMD_B, //icon 用 dummy
    BCMD_CROSS, //icon 用 dummy
    BCMD_ATTACK,
    BCMD_GET,
    BCMD_DOWN,
    BCMD_UP,
    BCMD_LOOT,
  };

private:
  typedef struct {
    int8_t duration;
    int8_t my;
    int8_t x, y;
    char v[6];
    ColorIndex c;
  } DISPNUM;

  static const int8_t MAX_DISPNUM = 16;

public:
  UICtrl();
  ~UICtrl();

  void update();
  void draw();


  //num
  inline void dispNumUp( ColorIndex c, int16_t v, int8_t x, int8_t y ) {
    entryDispNum( c, v, x, y, -2 );
  }
  inline void dispNumDown( ColorIndex c, int16_t v, int8_t x, int8_t y ) {
    entryDispNum( c, v, x, y, 2 );
  }


  /*
   * activate ButtonCommand
   * btn BCMD_EMPTY:not active   BCMD_A: button_a   BCMD_B: button_b
   */
  void openBtnCmd( int btn, uint8_t slotup, uint8_t slotright, uint8_t slotdown, uint8_t slotleft );
  void closeBtnCmd();
  inline uint8_t getBtnCmd( uint8_t slot ) {
    return m_btncmdslot[slot];
  }

private:
  void drawBtnCmd();

  //dispnum
  void entryDispNum( ColorIndex c, int16_t v, int8_t x, int8_t y, int8_t my );
//  void updateDispNum();
  void drawDispNum();

private:
  int8_t m_btncmd; //BCMD_EMPTY:not active   BCMD_A: button_a   BCMD_B: button_b
  uint8_t m_btncmdslot[BCSLOTMAX];
  DISPNUM m_dispnum[ MAX_DISPNUM ];
};

extern UICtrl g_uictrl;

#define UIC() g_uictrl

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
