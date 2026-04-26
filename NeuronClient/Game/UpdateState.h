#ifndef UpdateState_h__
#define UpdateState_h__

#include "Common.h"

struct UpdateState
{
  float dt;
  gametime_t quanta;
  bool hasFocus;

  UpdateState(float dt, bool hasFocus = true)
    : dt(dt),
      quanta(static_cast<gametime_t>((float)kTimeScale * dt)),
      hasFocus(hasFocus) {}
};

#endif
