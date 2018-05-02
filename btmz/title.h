#pragma once

#include "btmz.h"
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

class Title
{
private:
  enum {
    CUR_NEW,
    CUR_CONTINUE,
    CURMAX
  };

  enum {
    PHASE_MAINMENU,
    PHASE_DECIDE,
    PHASE_NEW,
  };

public:
  Title();
  ~Title();

  void update();
  void draw();

private:
  void updateMainMenu();
  void updateDecide();
  void updateNew();

  void drawMainMenu();
  void drawDecide();
  void drawNew();

private:
  int8_t m_cursor;
  int8_t m_phase;
  uint8_t m_flicker;
  bool m_existsave;
  bool m_active[CURMAX];
};





extern FLOWFUNCSET fsTitle;


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

