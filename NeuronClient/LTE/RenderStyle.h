#ifndef LTE_RenderStyle_h__
#define LTE_RenderStyle_h__

#include "Reference.h"

struct RenderStyleT : public RefCounted {
  virtual ~RenderStyleT() {}

  virtual void OnBegin() = 0;
  virtual void OnEnd() = 0;
  virtual void Render(Geometry const& geometry) = 0;
  virtual void SetShader(ShaderInstanceT* shader) = 0;
  virtual void SetTransform(Transform const& transform) = 0;
  virtual bool WillRender() const = 0;
};

RenderStyle RenderStyle_Get();
void RenderStyle_Pop();
void RenderStyle_Push(RenderStyle const&);

#endif
