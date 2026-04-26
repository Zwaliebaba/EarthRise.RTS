#ifndef LTE_StringPool_h__
#define LTE_StringPool_h__

#include "Map.h"
#include "String.h"

struct StringPool
{
  Map<String, const char*> strMap;

  const char* Get(const String& str)
  {
    const char*& ptr = strMap[str];
    if (!ptr)
      ptr = _strdup(str.c_str());
    return ptr;
  }
};

#endif
