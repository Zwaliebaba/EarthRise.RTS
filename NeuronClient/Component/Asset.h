#ifndef Component_Asset_h__
#define Component_Asset_h__

#include "Common.h"
#include "Game/Player.h"
#include "LTE/AutoClass.h"
#include "LTE/Pointer.h"

AutoClass(ComponentAsset, Player, owner)

  ComponentAsset() {}

  void Run(ObjectT* self, UpdateState& state);
};

AutoComponent(Asset)

  void OnUpdate(UpdateState& s)
  {
    Asset.Run(this, s);
    BaseT::OnUpdate(s);
  }

  const Player& GetOwner() const { return Asset.owner; }

  void SetOwner(const Player& owner)
  {
    if (Asset.owner)
      Asset.owner->RemoveAsset(this);
    owner->AddAsset(this);
  }
};

#endif
