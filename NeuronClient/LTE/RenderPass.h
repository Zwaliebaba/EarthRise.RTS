#ifndef LTE_RenderPass_h__
#define LTE_RenderPass_h__

#include "BaseType.h"
#include "Reference.h"
#include "DrawState.h"

struct RenderPassT : RefCounted
{
  BASE_TYPE(RenderPassT)

  void Render(DrawState* state);

  virtual const char* GetName() const = 0;
  virtual void OnRender(DrawState* state) = 0;
};

#endif
