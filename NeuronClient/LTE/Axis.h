#ifndef LTE_Axis_h__
#define LTE_Axis_h__

#include "BaseType.h"
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

  Axis Axis_MouseX();
  Axis Axis_MouseY();

  Axis Axis_MouseDragX(MouseButton button);
  Axis Axis_MouseDragY(MouseButton button);

  Axis Axis_MouseWheel();

  Axis Axis_Product(Axis const& a, Axis const& b);

  Axis Axis_Range(Axis const& source, float minValue, float maxValue);

  Axis Axis_Sum(Axis const& a, Axis const& b);
}

#endif
