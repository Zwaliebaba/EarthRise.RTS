#ifndef LTE_Mesh_h__
#define LTE_Mesh_h__

#include "AutoClass.h"
#include "Geometry.h"
#include "GLEnum.h"
#include "GLType.h"
#include "V3.h"
#include "Vector.h"
#include "Vertex.h"

AutoClassDerived(MeshT, GeometryT,
  Vector<Vertex>, vertices,
  Vector<uint>, indices)
  DERIVED_TYPE_EX(MeshT)

  mutable GL_Buffer vbo;
  mutable GL_Buffer ibo;
  mutable GL_IndexFormat::Enum indexFormat;
  mutable short bufferVersion;
  short version;

  MeshT() :
    vbo(GL_NullBuffer),
    ibo(GL_NullBuffer),
    bufferVersion(0),
    version(0)
    {}

  ~MeshT();

  /* Geometry. */
  void Draw() const;

  Bound3 GetBound() const;

  Mesh GetCollisionMesh() const;

  short GetVersion() const;

  bool Intersects(
    Ray const& r,
    float* tOut = nullptr,
    V3* normalOut = nullptr,
    V2* uvOut = nullptr) const;

  V3 Sample() const;

  /* Vertex Manipulation. */
  Mesh AddMesh(Mesh const&);

  Mesh AddMesh(Mesh const& mesh, Matrix const& transform);

  Mesh AddQuad(uint i1, uint i2, uint i3, uint i4);

  Mesh AddQuadR(int i1, int i2, int i3, int i4);

  Mesh AddTriangle(uint i1, uint i2, uint i3);

  Mesh AddVertex(Vertex const& vertex);

  Mesh AddVertex(
    V3 const& position,
    V3 const& normal,
    float u,
    float v);

  Mesh Clear();

  Mesh Clone() const;

  /* Computation. */
  void ComputeDecomp(Vector<Mesh>& pieces) const;

  Mesh ComputeEdgeDistance(float edgeThresh);

  Mesh ComputeNormals();

  Mesh ComputePrincipleComponent() const;

  /* Queries. */
  uint* GetIndexPointer();
  uint const* GetIndexPointer() const;

  uint GetIndices() const;

  uint GetTriangles() const;

  Vertex* GetVertexPointer();
  Vertex const* GetVertexPointer() const;

  V3 const* GetVertexNormalPointer() const;

  float const* GetVertexTexCoordPointer() const;

  uint GetVertices() const;

  /* Geometric Operations. */
  Mesh RemoveDegeneracies(float minTriArea = 1e-5f) const;

  Mesh ReverseWinding();

  Mesh ShareVertices(float maxDistance = 1e-5f);

  Mesh SmoothNormals(float distanceFactor = 0.1f);

  Mesh UnshareVertices(float minDotProduct = 0.8f);

  /* Geometric Transformations. */
  Mesh Transform(Matrix const& m);

  Mesh Translate(V3 const& translation);
  Mesh TranslateToCenter();

  Mesh Rotate(V3 const& ypr);

  Mesh Scale(V3 const& scale);

  Mesh SetU(float u);
  Mesh SetV(float v);

  template <class VertexFNType>
  Mesh Map(VertexFNType& fn) {
    for (size_t i = 0; i < vertices.size(); ++i)
      fn(vertices[i]);
    return this;
  }
};

inline Mesh Mesh_Create() {
  return new MeshT;
}

#endif
