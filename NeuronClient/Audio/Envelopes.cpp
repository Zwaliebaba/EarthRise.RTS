#include "Signal.h"
#include "LTE/StdMath.h"

struct ASRImpl : Audio::Signal
{
  double a, s, r;

  ASRImpl(double a, double s, double r)
    : a(a),
      s(s),
      r(r) {}
};

namespace Audio
{
  Signal* ASR(double a, double s, double r) { return new ASRImpl(a, s, r); }
}
