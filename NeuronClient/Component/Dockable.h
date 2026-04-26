#ifndef Component_Dockable_h__
#define Component_Dockable_h__

#include "Common.h"
#include "Game/Item.h"
#include "Game/Object.h"
#include "LTE/AutoClass.h"
#include "LTE/Bound.h"
#include "LTE/Vector.h"

AutoClass(ComponentDockable, Vector<Bound3>, hangars, Vector<Bound3>, ports, int, docked, int, capacity)

  ComponentDockable()
    : docked(0),
      capacity(0) {}

  bool CanDock(ObjectT* self, const Object& docker);
  void Dock(ObjectT* self, const Object& docker);
  void Undock(ObjectT* self, const Object& docker);
};

AutoComponent(Dockable)

  void SetSupertype(const Item& type)
  {
    if (type->GetDocks())
    {
      Dockable.ports = *type->GetDocks();
      Dockable.hangars = Dockable.ports; // TODO
    }
    Dockable.capacity = type->GetDockCapacity();

    BaseT::SetSupertype(type);
  }

  bool CanDock(const Object& docker) { return Dockable.CanDock(this, docker); }

  void Dock(const Object& docker) { Dockable.Dock(this, docker); }

  void Undock(const Object& docker) { Dockable.Undock(this, docker); }
};

#endif
