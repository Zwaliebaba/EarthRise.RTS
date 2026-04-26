#ifndef LTE_Axis_h__
#define LTE_Axis_h__

#include "BaseType.h"
#include "Joystick.h"
#include "Reference.h"
#include "String.h"

namespace LTE {
  struct AxisT : public RefCounted {
    BASE_TYPE(AxisT)
    float zeroValue;
    float maxValue;
    float deadZone;
    float power;
    bool invert;

    AxisT() :
      zeroValue(0),
      maxValue(1),
      deadZone(0),
      power(1),
      invert(false)
      {}

    float Get() const;

    virtual float GetRaw() const = 0;

    Axis Invert() {
      invert = !invert;
      return this;
    }

    Axis SetDeadZone(float deadZone) {
      this->deadZone = deadZone;
      return this;
    }

    Axis SetPower(float power) {
      this->power = power;
      return this;
    }

    Axis SetZeroValue(float zeroValue) {
      this->zeroValue = zeroValue;
      return this;
    }

    FIELDS {
      MAPFIELD(zeroValue)
      MAPFIELD(maxValue)
      MAPFIELD(deadZone)
      MAPFIELD(power)
      MAPFIELD(invert)
    }
  };

  Axis Axis_Button(Button const& button, bool inverted = false);

  Axis Axis_Capture();

  Axis Axis_Joy(uint joystickIndex, JoystickAxis axix);

  Axis Axis_MouseX();
  Axis Axis_MouseY();

  Axis Axis_MouseDragX(MouseButton button);
  Axis Axis_MouseDragY(MouseButton button);

  Axis Axis_MouseWheel();

  Axis Axis_Product(Axis const& a, Axis const& b);

  Axis Axis_Range(Axis const& source, float minValue, float maxValue);

  Axis Axis_Sum(Axis const& a, Axis const& b);

  inline Axis Axis_DpadVertical() {
    return Axis_Joy(0, JoystickAxis_PovX)->Invert();
  }

  inline Axis Axis_DpadHorizontal() {
    return Axis_Joy(0, JoystickAxis_PovY)->Invert();
  }

  inline Axis Axis_LeftStickX() {
    return Axis_Joy(0, JoystickAxis_X);
  }

  inline Axis Axis_LeftStickY() {
    return Axis_Joy(0, JoystickAxis_Y);
  }

  inline Axis Axis_LeftTrigger() {
    return Axis_Range(Axis_Joy(0, JoystickAxis_Z)->SetDeadZone(.2f), 0.f, 1.f);
  }

  inline Axis Axis_RightStickX() {
    return Axis_Joy(0, JoystickAxis_U);
  }

  inline Axis Axis_RightStickY() {
    return Axis_Joy(0, JoystickAxis_R);
  }

  inline Axis Axis_RightTrigger() {
    return Axis_Range(Axis_Joy(0, JoystickAxis_Z)->SetDeadZone(.2f), 0.f, -1.f);
  }
}

#endif
