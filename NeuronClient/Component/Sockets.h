#ifndef Component_Sockets_h__
#define Component_Sockets_h__

#include "Common.h"
#include "Game/Socket.h"
#include "Game/Item.h"
#include "Game/Object.h"
#include "LTE/Array.h"
#include "LTE/AutoClass.h"
#include "LTE/Pointer.h"
#include "LTE/Vector.h"

AutoClass(ComponentSockets,
  Array<Socket>, sockets,
  Array<Object>, instances)

  ComponentSockets() {}

  bool Plug(ObjectT* self, Item const& type);
  bool Plug(ObjectT* self, Item const& type, uint slot);
  bool Plug(ObjectT* self, Object const& type);
  bool Plug(ObjectT* self, Object const& object, uint slot);
  void Unplug(ObjectT* self, uint slot);
};

AutoComponent(Sockets)
  void SetSupertype(Item const& type) {
    DEBUG_ASSERT(type->GetSockets());
    Sockets.sockets = *type->GetSockets();
    Sockets.instances.resize(Sockets.sockets.size(), nullptr);

    BaseT::SetSupertype(type);
  }

  Capability GetCapability() const {
    Capability total = BaseT::GetCapability();
    for (size_t i = 0; i < Sockets.instances.size(); ++i)
      if (Sockets.instances[i])
        total += Sockets.instances[i]->GetCapability();
    return total;
  }

  Quantity GetValue() const {
    Quantity total = BaseT::GetValue();
    for (size_t i = 0; i < Sockets.instances.size(); ++i)
      if (Sockets.instances[i])
        total += Sockets.instances[i]->GetValue();
    return total;
  }

  bool Plug(Item const& item) {
    return Sockets.Plug(this, item);
  }

  bool Plug(Object const& object) {
    return Sockets.Plug(this, object);
  }
};

#endif
