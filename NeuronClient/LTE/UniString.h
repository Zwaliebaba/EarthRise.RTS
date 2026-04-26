#ifndef LTE_UniString_h__
#define LTE_UniString_h__

#include "Common.h"

struct UniStringIterator {
  uint32 codepoint;
  char const* iter;
  char const* end;

  void Advance();

  bool HasMore() const {
    return iter != end;
  }

  uint32 Get() const {
    return codepoint;
  }
};

UniStringIterator UniString_Begin(String const& source);

#endif
