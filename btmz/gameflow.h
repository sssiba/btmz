#pragma once

#include "btmz.h"


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
typedef void (*FlowInitFunc)(void);
typedef void (*FlowUpdateFunc)(void);
typedef void (*FlowDrawFunc)(void);
typedef void (*FlowFinishFunc)(void);

typedef struct {
  FlowInitFunc initfunc;
  FlowUpdateFunc updatefunc;
  FlowDrawFunc drawfunc;
  FlowFinishFunc finishfunc;
} FLOWFUNCSET;



//-------------------------------------------
//-------------------------------------------
//-------------------------------------------
struct GameFlow {
private:
  const FLOWFUNCSET* funcs;
  const FLOWFUNCSET* nextfuncs;

public:
  void setup() {
    funcs = NULL;
    nextfuncs = NULL;
  }
  void cleanup() {
    
  }

  void setFlow( const FLOWFUNCSET* fs ) {
    nextfuncs = fs;
  }

  // 別 flow への切り替え要求がされていれば true
  inline bool isRequestChange() {
    return nextfuncs != NULL;
  }

  void update() {
    if( nextfuncs ) {
      if( funcs ) {
        if( funcs->finishfunc ) funcs->finishfunc();
      }
      funcs = nextfuncs;
      if( funcs->initfunc ) funcs->initfunc();
      nextfuncs = NULL;
    }
    if( !funcs ) return;
    
    if( funcs->updatefunc ) funcs->updatefunc();
  }
  void draw() {
    if( !funcs ) return;
    if( funcs->drawfunc ) funcs->drawfunc();
  }
};

//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

