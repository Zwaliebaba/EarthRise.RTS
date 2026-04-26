#ifndef LTE_Model_h__
#define LTE_Model_h__

#include "Geometry.h"
#include "Renderable.h"

struct ModelT : public RenderableT {
  DERIVED_TYPE_EX(ModelT)
  struct ModelData& d;
  short version;

  ModelT();
  ~ModelT();

  size_t GetHash() const {
    return (size_t)this;
  }

  short GetVersion() const {
    return version;
  }

  Model Add(
    Geometry const& geom,
    ShaderInstance const& shader,
    bool collidable = true);

  Model Add(Model const& model);

  void Render(DrawState* state) const;
  
  void RenderPiece(size_t piece, DrawState* state) const;

  Bound3 GetBound() const;
  
  Mesh GetCollisionMesh() const;

  bool Intersects(
    Ray const& r,
    float* tOut = nullptr,
    V3* normalOut = nullptr) const;
  
  V3 Sample() const;

  /* NOTE : Need to call UpdateVersion every time you change one of the internal
            rebderables once it is already added to the model! Model cannot
            automatically tell that it has changed. */
  Model UpdateVersion();
};

inline Model Model_Create() {
  return new ModelT;
}

#endif
