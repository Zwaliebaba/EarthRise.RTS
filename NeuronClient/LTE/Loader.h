#ifndef LTE_Loader_h__
#define LTE_Loader_h__

#include "Common.h"

namespace LTE {
  bool RegisterLoader(void (*loader)());
  bool RegisterUnloader(void (*unloader)());
}

#endif
