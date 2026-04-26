#ifndef LTE_Renderable_h__
#define LTE_Renderable_h__

#include "BaseType.h"
#include "Bound.h"
#include "Reference.h"
#include "V3.h"
#include "LTE/DrawState.h"

struct RenderableT : RefCounted
{
  BASE_TYPE(RenderableT)

  virtual Bound3 GetBound() const { return Bound3(0); }

  virtual Mesh GetCollisionMesh() const;

  virtual size_t GetHash() const { return 0; }

  virtual short GetVersion() const { return 0; }

  virtual bool Intersects(const Ray& r, float* tOut = nullptr, V3* normalOut = nullptr) const
  {
    NOT_IMPLEMENTED
    return false;
  }

  virtual void Render(DrawState* state) const = 0;

  virtual V3 Sample() const { return 0; }

  FIELDS {}
};

Renderable Renderable_Union(const Renderable& a, const Renderable& b);

#endif
