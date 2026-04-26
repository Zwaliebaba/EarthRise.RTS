#ifndef Module_PhysicsEngine_h__
#define Module_PhysicsEngine_h__

#include "Common.h"
#include "LTE/Module.h"
#include "Game/Common.h"

struct PhysicsEngine : public ModuleT {
  PhysicsEngine();
  virtual ~PhysicsEngine();

  void Push();
  void Pop();

  virtual bool CheckCollision(
    ObjectT* object1,
    ObjectT* object2,
    V3* contactNormal = nullptr) = 0;

  virtual bool Raycast(
    WorldRay const& ray,
    ObjectT* object,
    float tMax,
    float& tOut,
    V3* normalOut = nullptr) = 0;
};

PhysicsEngine* CreatePhysicsEngine();
PhysicsEngine* CreatePhysicsEngineNull();
PhysicsEngine* GetPhysicsEngine();

#endif
