#ifndef Component_Queryable_h__
#define Component_Queryable_h__

#include "Common.h"
#include "LTE/AutoClass.h"
#include "LTE/Pointer.h"
#include "LTE/V3.h"

struct ComponentQueryableImpl;

AutoClassEmpty(ComponentQueryable)
  Pointer<ComponentQueryableImpl> impl;

  ComponentQueryable();
  ~ComponentQueryable();

  void Remove(ObjectT* object);

  void Run(ObjectT* self, UpdateState& state);

  void QueryBox(
    ObjectT* self,
    Bound3D const& box,
    Vector<ObjectT*>& objects);

  ObjectT* QueryRay(
    ObjectT* self,
    WorldRay const& ray,
    float& t,
    float tMax,
    V3* normalOut,
    bool accel,
    bool (*check)(ObjectT const*, void*),
    void* aux);
};

AutoComponent(Queryable)
  void OnUpdate(UpdateState& s) {
    Queryable.Run(this, s);
    BaseT::OnUpdate(s);
  }

  void QueryInterior(
    Bound3D const& box,
    Vector<ObjectT*>& objects)
  {
    return Queryable.QueryBox(this, box, objects);
  }

  ObjectT* QueryInterior(
    WorldRay const& ray,
    float& t,
    float tMax,
    V3* normalOut,
    bool accel,
    bool (*check)(ObjectT const*, void*),
    void* aux)
  {
    return Queryable.QueryRay(this, ray, t, tMax, normalOut, accel, check, aux);
  }
};

/* Pre-defined callbacks for use with Query. */
bool RaycastCanCollide(ObjectT const*, void*);
bool RaycastCanCollideBidirectional(ObjectT const*, void*);
bool RaycastSolids(ObjectT const*, void*);

#endif
