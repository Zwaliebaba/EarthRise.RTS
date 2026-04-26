#include "Cursor.h"
#include "LTE/AutoClass.h"

#include <vector>

namespace {
  AutoClass(Cursor,
    V2, pos,
    V2, last)
    Cursor() {}
  };

  std::vector<Cursor>& GetStack() {
    static std::vector<Cursor> stack;
    return stack;
  }
}

DefineFunction(Cursor_Get) {
  return GetStack().back().pos;
}

DefineFunction(Cursor_GetDelta) {
  Cursor const& cursor = GetStack().back();
  return cursor.pos - cursor.last;
}

DefineFunction(Cursor_GetLast) {
  return GetStack().back().last;
}

DefineFunction(Cursor_Pop) {
  GetStack().pop_back();
}

DefineFunction(Cursor_Push) {
  GetStack().push_back(Cursor(args.pos, args.posLast));
}
