#ifndef Component_Attachable_h__
#define Component_Attachable_h__

#include "Common.h"
#include "Game/Item.h"
#include "LTE/AutoClass.h"
#include "LTE/Transform.h"

AutoClass(ComponentAttachable, Transform, transform, short, parentVersion, bool, moved)

  ComponentAttachable()
    : parentVersion(0),
      moved(false) {}

  void SetPos(const Position& pos)
  {
    moved = true;
    transform.pos = pos;
  }

  void SetLook(const V3& look)
  {
    moved = true;
    transform.look = look;
  }

  void SetScale(const V3& scale)
  {
    moved = true;
    transform.scale = scale;
  }

  void SetTransform(const Transform& transform)
  {
    moved = true;
    this->transform = transform;
  }

  void SetUp(const V3& up)
  {
    moved = true;
    transform.up = up;
  }

  void Run(ObjectT* self, UpdateState& state) { UpdateTransform(self); }

  void UpdateTransform(ObjectT* self);
};

AutoComponent(Attachable)

  void OnUpdate(UpdateState& s)
  {
    Attachable.Run(this, s);
    BaseT::OnUpdate(s);
  }

  const Transform& GetLocalTransform() const { return Attachable.transform; }

  void SetSupertype(const Item& type)
  {
    if (type->GetScale() > 0)
      Attachable.SetScale(V3(type->GetScale()));

    BaseT::SetSupertype(type);
  }

  void SetLocalTransform(const Transform& transform) { Attachable.SetTransform(transform); }
};

#endif
