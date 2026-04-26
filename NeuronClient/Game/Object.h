#ifndef Game_Object_h__
#define Game_Object_h__

#include "Capability.h"
#include "Signature.h"
#include "LTE/DrawState.h"
#include "AI/Types.h"
#include "LTE/BaseType.h"
#include "LTE/Data.h"
#include "LTE/Pointer.h"
#include "LTE/Reference.h"
#include "LTE/V3.h"

#include "UI/Common.h"

struct ObjectT : RefCounted
{
  BASE_TYPE_EX(ObjectT)

  ObjectID id;
  Pointer<ObjectT> parent;
  Pointer<ObjectT> container;
  Object nextSibling;
  Object children;
  bool deleted;

  ObjectT();
  ~ObjectT() override;

  /* Common Operations. */
  bool CanMove() const;

  void Delete();

  virtual Capability GetCapability() const;

  Pointer<ObjectT> GetRoot();

  Pointer<const ObjectT> GetRoot() const;

  /* Pure. */
  virtual ObjectType GetType() const = 0;

  /* Inline. */
  Pointer<ObjectT> GetContainer() const
  {
    if (container)
      return container;
    return parent ? parent->GetContainer() : nullptr;
  }

  HashT GetHash() const { return static_cast<HashT>(id % HASHT_MAX); }

  ObjectID GetID() const { return id; }

  const char* GetTypeString() const { return ObjectType_String[GetType()]; }

  bool IsDeleted() const { return deleted; }

  /* Non-Component Related. */
  virtual float GetCooldown() const { return -1; }

  virtual Quantity GetMaxUses() const { return 0; }

  virtual float GetRange() const { return 0; }

  virtual uint GetSeed() const { return 0; }

  virtual Signature GetSignature() const { return Signature(0, 0, 0, 0); }

  virtual Quantity GetUses() const { return 0; }

  virtual Icon GetIcon() const;

  virtual Widget GetWidget(const Player& self);

  float GetMaxRange() const;

  float GetMinRange() const;

  virtual const Traits& GetTraits() const;

  virtual Quantity GetValue() const;

  virtual void OnCreate();

  virtual void OnDeath();

  virtual void OnDestroy();

  virtual void OnUpdate(UpdateState& state) {}

  void Update(UpdateState& state);

  /* Drawing. */
  virtual void OnDraw(DrawState* state) {}
  virtual void OnDrawInterior(DrawState* state) {}
  virtual void BeginDrawInterior(DrawState* state) {}
  virtual void EndDrawInterior(DrawState* state) {}

  /* Account. */
  virtual void AddCredits(Quantity count) { NOT_IMPLEMENTED }

  virtual Quantity GetCredits() const { return 0; }

  virtual bool RemoveCredits(Quantity count)
  {
    NOT_IMPLEMENTED
    return false;
  }

  /* Asset. */
  virtual const Player& GetOwner() const;

  virtual void SetOwner(const Player&) { NOT_IMPLEMENTED }

  /* Assets. */
  virtual void AddAsset(const Object& asset) { NOT_IMPLEMENTED }

  virtual void RemoveAsset(const Object& asset) { NOT_IMPLEMENTED }

  /* Attachable. */
  virtual const Transform& GetLocalTransform() const;

  virtual void SetLocalTransform(const Transform& transform) { NOT_IMPLEMENTED }

  /* BoundingBox. */
  Position GetCenter() const;
  V3 GetExtent() const;
  float GetRadius() const;

  /* Cargo. */
  virtual bool AddItem(const Item& item, Quantity quantity, bool force = false) { return false; }

  bool RemoveItem(const Item& item, Quantity quantity) { return AddItem(item, -quantity); }

  Mass GetFreeCapacity() const { return GetCapability().Storage - GetUsedCapacity(); }

  virtual Mass GetUsedCapacity() const { return 0; }

  virtual Quantity GetItemCount(const Item& item) const { return 0; }

  /* Collidable. */
  virtual bool CanCollide(const ObjectT* object) const { return true; }

  virtual void OnCollide(ObjectT* self, ObjectT* other, const Position& selfLocation, const Position& otherLocation) {}

  /* Container. */
  virtual void AddInterior(const Object& child) { NOT_IMPLEMENTED }

  virtual void RemoveInterior(const Object& child) { NOT_IMPLEMENTED }

  /* Cullable. */
  virtual float GetCullDistanceMult() const { return 1.0f; }

  Position GetDockLocation(const Object& docker) const;

  /* Dockable. */
  virtual bool CanDock(const Object& docker) { return false; }

  virtual void Dock(const Object& docker) { NOT_IMPLEMENTED }

  virtual void Undock(const Object& docker) { NOT_IMPLEMENTED }

  /* Drawable. */
  Bound3 GetLocalBound() const;
  Bound3D GetGlobalBound() const;

  virtual Renderable GetRenderable() const;

  Pointer<ObjectT> GetContainerRoot() const;
  Pointer<const ObjectT> GetRegion() const;
  Pointer<const ObjectT> GetSystem() const;
  Pointer<const Universe> GetUniverse() const;

  /* Integrity. */
  virtual ItemT* GetDataDamaged() const { return nullptr; }

  virtual ItemT* GetDataDestroyed() const { return nullptr; }

  virtual Health GetHealth() const { return 0; }

  virtual Health GetMaxHealth() const { return 0; }

  virtual float GetHealthNormalized() const
  {
    Health maxHealth = GetMaxHealth();
    return maxHealth > 0 ? static_cast<float>(GetHealth()) / static_cast<float>(maxHealth) : 0;
  }

  bool IsAlive() const { return GetHealth() > 0; }

  Health GetDamage() const { return GetMaxHealth() - GetHealth(); }

  Health GetTotalDamage() const { return GetTotalMaxHealth() - GetTotalHealth(); }

  Damage ApplyDamage(Damage damage);
  Health GetTotalHealth() const;
  Health GetTotalMaxHealth() const;

  /* Log. */
  virtual void AddLogMessage(const String& message, float importance = 0) { NOT_IMPLEMENTED }

  /* Market. */
  virtual void AddMarketAsk(const Order&) { NOT_IMPLEMENTED }

  virtual void AddMarketBid(const Order&) { NOT_IMPLEMENTED }

  virtual void RemoveMarketAsk(const Order&) { NOT_IMPLEMENTED }

  virtual void RemoveMarketBid(const Order&) { NOT_IMPLEMENTED }

  /* Messaging. */
  virtual void Broadcast(Data& message);

  virtual void OnMessage(Data& message) {}

  template <class T>
  void Broadcast(const T& value)
  {
    Data message(value);
    Broadcast(message);
    Mutable(value) = message.Convert<T>();
  }

  template <class T>
  void Send(const T& value)
  {
    Data message(value);
    Send(message);
    Mutable(value) = message.Convert<T>();
  }

  void Send(Data& message) { OnMessage(message); }

  /* MissionBoard. */
  virtual void AddMissionListing(const Mission&) { NOT_IMPLEMENTED }

  virtual void RemoveMissionListing(const Mission&) { NOT_IMPLEMENTED }

  /* Missions. */
  virtual void AddMission(const Mission&) { NOT_IMPLEMENTED }

  virtual void RemoveMission(const Mission&) { NOT_IMPLEMENTED }

  /* Motion. */
  virtual void ApplyForce(const V3&) { NOT_IMPLEMENTED }

  virtual void ApplyTorque(const V3&) { NOT_IMPLEMENTED }

  virtual V3 GetAngularVelocity() const { return 0; }

  virtual Mass GetMass() const { return 1000000; }

  virtual float GetSpeed() const { return 0; }

  virtual float GetTopSpeed() const { return 0; }

  virtual V3 GetVelocity() const { return 0; }

  virtual V3 GetVelocityA() const { return 0; }

  float GetImpactTime(const ObjectT* source, float speed, Position& hitPoint) const;

  virtual bool IsMovable() const { return false; }

  /* Nameable. */
  virtual String GetName() const { return GetTypeString(); }

  virtual void SetName(const String& name) { NOT_IMPLEMENTED }

  /* Parent. */
  void AddChild(const Object& child);

  void Attach(const Object& child, const Transform& transform);

  void RemoveChild(const Object& child);

  /* Pluggable. */
  virtual float GetPowerFraction() const { return 0; }

  /* Supertyped. */
  virtual ItemT* GetSupertype() const { return nullptr; }

  virtual void SetSupertype(const Item&) {}

  /* Opinion. */
  virtual float GetOpinion(const Object&) const { return 0.0f; }

  virtual void ModOpinion(const Object&, float) {}

  bool Dislikes(const Object&) const;

  bool Likes(const Object&) const;

  /* Orientation. */
  V3 GetLook() const;

  V3 GetUp() const;

  V3 GetRight() const;

  Position GetPos() const;

  V3 GetScale() const;

  virtual const Transform& GetTransform() const;

  virtual void SetLook(const V3& look) { NOT_IMPLEMENTED }

  virtual void SetPos(const Position& position) { NOT_IMPLEMENTED }

  virtual void SetScale(const V3& scale) { NOT_IMPLEMENTED }

  virtual void SetUp(const V3& up) { NOT_IMPLEMENTED }

  /* Queryable. */
  virtual void QueryInterior(const Bound3D& box, Vector<ObjectT*>& objects) {}

  virtual ObjectT* QueryInterior(const WorldRay& ray, float& t, float tMax, V3* normalOut = nullptr, bool accelerate = false,
                                 bool (*check)(const ObjectT*, void*) = nullptr, void* aux = nullptr) { return nullptr; }

  /* Scriptable. */
  virtual void AddScript(const Data& object) { NOT_IMPLEMENTED; }

  /* Sockets. */
  virtual bool Plug(const Item& item)
  {
    NOT_IMPLEMENTED
    return false;
  }

  virtual bool Plug(const Object& object)
  {
    NOT_IMPLEMENTED
    return false;
  }

  /* Storage. */
  virtual Object GetStorageLocker(const Object& owner) { return nullptr; }

  /* Tasks. */
  virtual void ClearTasks() { NOT_IMPLEMENTED }

  virtual const TaskInstance* GetCurrentTask() const { return nullptr; }

  virtual void PushTask(const Task&) { NOT_IMPLEMENTED }

  /* Thrustable. */
  float GetMaxThrust() const { return GetCapability().Motion; }

  float GetMaxTorque() const;

  bool InRange(const ObjectT* other, Distance distance, bool precise = false) const;

  /* Component Access. */
  bool HasComponent(ComponentType type) const
  {
    switch (type)
    {
#define X(x) case ComponentType_##x: return Get##x() != nullptr;
    COMPONENT_X
#undef X

    default:
      return false;
    }
  }

#define X(x) virtual Pointer<Component##x> Get##x() {                        \
    return nullptr;                                                            \
  }
  COMPONENT_X
#undef X

#define X(x) Pointer<const Component##x> Get##x() const {                    \
    return ((ObjectT*)this)->Get##x().get();                                   \
  }
  COMPONENT_X
#undef X

  FIELDS
  {
    MAPFIELD(id)
    MAPFIELD(parent)
    MAPFIELD(container)
    MAPFIELD(nextSibling)
    MAPFIELD(children)
    MAPFIELD(deleted)
  }
};

#endif
