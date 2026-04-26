#include "RenderStyle.h"

#include <vector>

namespace {
  std::vector<RenderStyle>& GetStack() {
    static std::vector<RenderStyle> stack;
    return stack;
  }
}

RenderStyle RenderStyle_Get() {
  return GetStack().size() ? GetStack().back() : nullptr;
}

void RenderStyle_Pop() {
  GetStack().back()->OnEnd();
  GetStack().pop_back();
}

void RenderStyle_Push(RenderStyle const& style) {
  GetStack().push_back(style);
  style->OnBegin();
}
