#ifndef LTE_Finite_h__
#define LTE_Finite_h__

#include "Common.h"

namespace LTE
{
  template <class T>
  bool IsFinite(const T& t) { return true; }

  template <>
  inline bool IsFinite<float>(const float& t) { return t <= FLT_MAX && t >= -FLT_MAX; }

  template <>
  inline bool IsFinite<double>(const double& t) { return t <= DBL_MAX && t >= -DBL_MAX; }
}

#endif
