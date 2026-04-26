#include "Camera.h"
#include "UpdateState.h"
#include "Component/Motion.h"
#include "LTE/Bound.h"
#include "LTE/Math.h"
#include "LTE/Mouse.h"
#include "LTE/Smooth.h"
#include "LTE/Vector.h"
#include "LTE/View.h"
#include "LTE/Viewport.h"

constexpr float FOV_Y = XMConvertToRadians(62.0f);
constexpr float Z_NEAR = 0.05f;
constexpr float Z_FAR = 1.0e6f;

namespace
{
  Vector<Camera>& GetStack()
  {
    static Vector<Camera> stack;
    return stack;
  }

  AutoClassDerived(CameraImpl, CameraT, float, fov, Position, relativePos, Position, relativeLookAt, V3, relativeUp, Smooth<Position>,
                   position, Smooth<Position>, lookAt, Smooth<V3>, up, Object, target, float, rigidity)

    DERIVED_TYPE_EX(CameraImpl)

    CameraImpl()
      : fov(Radians(62.0f)),
        relativePos(0, 1, -10),
        relativeLookAt(0, 0, 0),
        relativeUp(0, 1, 0),
        position(Position(0, 0, -1)),
        lookAt(Position(0)),
        up(V3(0, 1, 0)),
        rigidity(1)
    {
      Motion.mass = 1;
      Motion.inertia = 1;
    }

    void OnUpdate(UpdateState& state) override
    {
      BaseType::OnUpdate(state);

      if (target && !target->IsDeleted())
      {
        target->GetContainer()->AddInterior(this);
        //Position offset = target->GetLocalBound().GetCenter();
        //Transform const& transform = target->GetTransform();
        position.target = relativePos; // + transform.TransformPoint(offset);
        lookAt.target = relativeLookAt; // + transform.TransformPoint(offset);
        up.target = relativeUp;

        Motion.mass = target->GetMass();
        Motion.velocity = target->GetVelocity();
      }

      position.value += Motion.velocity * state.dt;
      position.Update(rigidity * state.dt);
      lookAt.Update(rigidity * state.dt);
      up.Update(rigidity * state.dt);

      V3 newLook = Normalize(lookAt.value - position.value);
      Orientation.GetTransformW() = Transform_LookUp(position, newLook, up);
    }

    const Object& GetTarget() const override { return target; }

    View GetView(float aspect) const override { return View(GetTransform(), fov, aspect, Z_NEAR, Z_FAR); }

    void SetFov(float fov) override { this->fov = fov; }

    void SetRelativePos(const V3& pos) override { relativePos = pos; }

    void SetRelativeLookAt(const V3& lookAt) override { relativeLookAt = lookAt; }

    void SetRelativeUp(const V3& up) override { relativeUp = up; }

    void SetRigidity(float rigidity) override { this->rigidity = rigidity; }

    void SetTarget(const Object& object) override { target = object; }
  };

  DERIVED_IMPLEMENT(CameraImpl)
}

DefineFunction(Camera_Create) { return new CameraImpl; }

DefineFunction(Camera_CanSee)
{
  Camera cam = Camera_Get();
  if (!cam)
    return false;
  float d = Squared(args.maxDistance);
  return LengthSquared(args.object->GetPos() - cam->GetPos()) < d;
}

DefineFunction(Camera_Get) { return GetStack().size() ? GetStack().back() : nullptr; }

void Camera_Pop() { GetStack().pop(); }

void Camera_Push(const Camera& camera) { GetStack().push(camera); }
