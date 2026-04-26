#ifndef Component_Assets_h__
#define Component_Assets_h__

#include "Common.h"
#include "Game/Capability.h"
#include "Game/Object.h"
#include "LTE/AutoClass.h"
#include "LTE/Vector.h"

AutoClass(ComponentAssets, Vector<Object>, elements)

  ComponentAssets() {}

  ~ComponentAssets();

  void Add(ObjectT* self, const Object& asset);
  void Remove(ObjectT* self, const Object& asset);
};

AutoComponent(Assets)
  void AddAsset(const Object& asset) { Assets.Add(this, asset); }

  void RemoveAsset(const Object& asset) { Assets.Remove(this, asset); }

  Capability GetCapability() const
  {
    Capability total = BaseT::GetCapability();
    for (size_t i = 0; i < Assets.elements.size(); ++i)
      total += Assets.elements[i]->GetCapability();
    return total;
  }
};

#endif
