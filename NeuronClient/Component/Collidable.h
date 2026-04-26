#ifndef Component_Collidable_h__
#define Component_Collidable_h__

#include "Common.h"
#include "LTE/AutoClass.h"

AutoClass(ComponentCollidable, bool, passive, bool, solid)

  ComponentCollidable();

  void CheckCollisions(ObjectT* self, UpdateState& state);

  void Collide(ObjectT* self, ObjectT* other, const Position& pSelf, const Position& pOther);

  void Run(ObjectT* self, UpdateState& state)
  {
    if (!passive)
      CheckCollisions(self, state);
  }
};

AutoComponent(Collidable)

  void OnUpdate(UpdateState& s)
  {
    Collidable.Run(this, s);
    BaseT::OnUpdate(s);
  }
};

#endif
