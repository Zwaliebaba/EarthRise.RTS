#ifndef LTE_Timer_h__
#define LTE_Timer_h__

#include "Common.h"

struct Timer
{
  struct TimerData* d;

  Timer();
  ~Timer();

  float Lap()
  {
    float elapsed = GetElapsed();
    Reset();
    return elapsed;
  }

  /* NOTE : Measured in seconds. */
  float GetElapsed() const;

  void Reset() const;
};

#endif
