#ifndef LTE_Geom_h__
#define LTE_Geom_h__

#include "Vector.h"

namespace LTE {

  Mesh Geom_Box(float sx = 1, float sy = 1, float sz = 1);

  Mesh Geom_Cylinder(uint slices);

  Mesh Geom_DisplacePlane(Mesh const& m, Plane const& p, float amount);

  Mesh Geom_Expand(Mesh const& m, V3 const& origin, float x, float y, float z);

  Mesh Geom_MakeSymmetric(Mesh const& m, V3 const& plane);

  Mesh Geom_MakeSymmetricAngular(Mesh const& m, uint count, V3 const& axis);

  Mesh Geom_Mirror(Mesh const& m, V3 const& mirrorPlane);

  void Geom_MirrorPointRing(Vector<V3>& pointRing);

  Mesh Geom_PolyBox(uint sides, float sx, float sy, float sz);

  Mesh Geom_Repeat(
    Mesh const& m,
    uint count,
    V3 const& direction,
    float spacing = 1);

  Mesh Geom_Retarget(Mesh const& m, Bound3 const& targetBox);

  Mesh Geom_RingMesh(
    uint slices,
    uint pointCount,
    float bevelSize,
    float bevelDepth,
    float ringSpacing,
    float ringMinWidth,
    float ringMaxWidth,
    float edgeBevel,
    bool symmetricRing);

  Mesh Geom_SmoothHull(Mesh const& source, uint quality, float radius);

  Mesh Geom_Sphere(uint stacks, uint slices);

}

#endif
