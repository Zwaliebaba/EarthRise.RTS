#ifndef LTE_SDF_h__
#define LTE_SDF_h__

#include "BaseType.h"
#include "Reference.h"
#include "String.h"
#include "V3.h"

struct SDFT : public RefCounted {
  BASE_TYPE(SDFT)

  virtual float Evaluate(V3 const& p) const = 0;

  virtual Bound3 GetBound() const = 0;

  virtual String GetCode() const {
    return GetCode("p");
  }

  virtual String GetCode(String const& p) const {
    return "0";
  }

  virtual V3 Gradient(V3 const& p) const;

  SDF Add(SDF const& other);
  
  SDF Intersect(SDF const& other, float sharpness = -1);
  
  SDF Subtract(SDF const& other, float sharpness = -1);
  
  SDF Union(SDF const& other, float sharpness = -1);

  SDF Expand(V3 const& amount);
  
  SDF ExpandRadial(V3 const& axis, float amount);
  
  SDF Mirror(V3 const& origin, V3 const& normal);

  SDF MirrorRadial(
    V3 const& origin,
    V3 const& normal,
    float angleSize);

  SDF Multiply(float value);
  
  SDF PinchAxis(V3 const& axis);
  
  SDF PinchY(V3 const& axis);
  
  SDF Repeat(V3 const& frequency, V3 const& spacing = 0);
  
  SDF Rotate(V3 const& ypr);
  
  SDF Scale(V3 const& s);
  
  SDF Translate(V3 const& t);

  FIELDS {}
};

#endif
