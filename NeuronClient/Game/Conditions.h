#ifndef Game_Conditions_h__
#define Game_Conditions_h__

#include "Common.h"

bool Condition_Nearby(
  Object const& a,
  Object const& b,
  Distance d = 0.0);

#endif
