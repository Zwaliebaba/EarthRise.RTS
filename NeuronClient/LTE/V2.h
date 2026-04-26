#ifndef LTE_V2_h__
#define LTE_V2_h__

#include "Common.h"
#include "AutoClass.h"
#include "Finite.h"
#include "StdMath.h"

template <class T>
AutoClass(V2T, T, x, T, y)

  using ReturnType = T;
  using MetricType = T;

  /* Constructors. */
  V2T() {}

  V2T(T xy)
    : x(xy),
      y(xy) {}

  template <class U>
  V2T(const V2T<U>& v)
    : x(static_cast<T>(v.x)),
      y(static_cast<T>(v.y)) {}

  T& operator[](int index) { return (&x)[index]; }

  const T& operator[](int index) const { return (&x)[index]; }

  bool operator==(const V2T& v) const { return x == v.x && y == v.y; }

  bool operator!=(const V2T& v) const { return x != v.x || y != v.y; }

  bool operator<(const V2T& v) const { return x < v.x && y < v.y; }

  bool operator<=(const V2T& v) const { return x <= v.x && y <= v.y; }

  bool operator>(const V2T& v) const { return x > v.x && y > v.y; }

  bool operator>=(const V2T& v) const { return x >= v.x && y >= v.y; }

  void operator+=(const V2T& v)
  {
    x += v.x;
    y += v.y;
  }

  void operator-=(const V2T& v)
  {
    x -= v.x;
    y -= v.y;
  }

  void operator*=(const V2T& v)
  {
    x *= v.x;
    y *= v.y;
  }

  void operator/=(const V2T& v)
  {
    x /= v.x;
    y /= v.y;
  }

  void operator*=(T f)
  {
    x *= f;
    y *= f;
  }

  void operator/=(T f)
  {
    x /= f;
    y /= f;
  }

  V2T operator-() const { return V2T(-x, -y); }

  friend V2T operator+(const V2T& v1, const V2T& v2) { return V2T(v1.x + v2.x, v1.y + v2.y); }

  friend V2T operator-(const V2T& v1, const V2T& v2) { return V2T(v1.x - v2.x, v1.y - v2.y); }

  friend V2T operator*(const V2T& v1, const V2T& v2) { return V2T(v1.x * v2.x, v1.y * v2.y); }

  friend V2T operator/(const V2T& v1, const V2T& v2) { return V2T(v1.x / v2.x, v1.y / v2.y); }

  friend V2T operator*(const V2T& v1, T s) { return V2T(v1.x * s, v1.y * s); }

  friend V2T operator/(const V2T& v1, T s) { return V2T<T>(v1.x / s, v1.y / s); }

  friend V2T operator*(T s, const V2T& v1) { return V2T(s * v1.x, s * v1.y); }

  friend V2T operator/(T s, const V2T& v1) { return V2T(s / v1.x, s / v1.y); }

  /* Member functions. */
  bool IsFinite() const { return LTE::IsFinite(x) && LTE::IsFinite(y); }

  T GetAverage() const { return (x + y) / 3.0f; }

  T GetGeometricAverage() const { return Sqrt(x * y); }

  T GetLengthSquared() const { return x * x + y * y; }

  T GetLength() const { return Sqrt(GetLengthSquared()); }

  T GetMax() const { return x >= y ? x : y; }

  T GetMin() const { return x <= y ? x : y; }

  T GetProduct() const { return x * y; }

  T GetSum() const { return x + y; }

  V2T GetX() const { return V2T(x, 0); }

  V2T GetY() const { return V2T(0, y); }

  /* Vectorized standard math. */
  friend V2T Abs(const V2T& v) { return V2T(Abs(v.x), Abs(v.y)); }

  friend V2T Ceil(const V2T& v) { return V2T(Ceil(v.x), Ceil(v.y)); }

  friend V2T Clamp(const V2T& v, const V2T& lower, const V2T& upper)
  {
    return V2T(Clamp(v.x, lower.x, upper.x), Clamp(v.y, lower.y, upper.y));
  }

  friend MetricType Dot(const V2T& v1, const V2T& v2) { return v1.x * v2.x + v1.y * v2.y; }

  friend V2T Exp(const V2T& v) { return V2T(Exp(v.x), Exp(v.y)); }

  friend V2T Floor(const V2T& v) { return V2T(Floor(v.x), Floor(v.y)); }

  friend V2T Fract(const V2T& v) { return V2T(Fract(v.x), Fract(v.y)); }

  friend MetricType Length(const V2T& v) { return Sqrt(Dot(v, v)); }

  friend MetricType LengthSquared(const V2T& v) { return Dot(v, v); }

  friend V2T Log(const V2T& v) { return V2T(Log(v.x), Log(v.y)); }

  friend V2T Max(const V2T& v1, const V2T& v2) { return V2T(Max(v1.x, v2.x), Max(v1.y, v2.y)); }

  friend V2T Min(const V2T& v1, const V2T& v2) { return V2T(Min(v1.x, v2.x), Min(v1.y, v2.y)); }

  friend V2T Mix(const V2T& v1, const V2T& v2, const V2T& t) { return V2T(Mix(v1.x, v2.x, t.x), Mix(v1.y, v2.y, t.y)); }

  friend V2T Normalize(const V2T& v)
  {
    MetricType l = Length(v);
    return l > 0 ? v / l : v;
  }

  friend MetricType PNorm(const V2T& v, T p) { return Pow(Pow(Abs(v.x), p) + Pow(Abs(v.y), p), 1 / p); }

  friend V2T Pow(const V2T& v1, const V2T& v2) { return V2T(Pow(v1.x, v2.x), Pow(v1.y, v2.y)); }

  friend V2T Round(const V2T& v) { return V2T(Round(v.x), Round(v.y)); }

  friend V2T Saturate(const V2T& v) { return V2T(Saturate(v.x), Saturate(v.y)); }

  friend V2T Sign(const V2T& v) { return V2T(Sign(v.x), Sign(v.y)); }
};

LT_API V2 RandV2(float min, float max);

#endif
