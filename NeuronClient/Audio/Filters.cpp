#include "Signal.h"

#include "LTE/AutoPtr.h"
#include "LTE/StdMath.h"

namespace Audio { namespace
  {
    struct ProductImpl : SignalT
    {
      Signal a;
      Signal b;

      ProductImpl(const Signal& a, const Signal& b)
        : a(a),
          b(b) {}

      double OnGet(const GlobalData& d) override { return a->Get(d) * b->Get(d); }
    };

    struct SumImpl : SignalT
    {
      Signal a;
      Signal b;
      double mixA;
      double mixB;

      SumImpl(const Signal& a, const Signal& b, double mixA, double mixB)
        : a(a),
          b(b),
          mixA(mixA),
          mixB(mixB) {}

      double OnGet(const GlobalData& d) override { return mixA * a->Get(d) + mixB * b->Get(d); }
    };
  }

  Signal Signal_Product(const Signal& a, const Signal& b) { return new ProductImpl(a, b); }

  Signal Signal_Sum(const Signal& a, const Signal& b, double mixA, double mixB) { return new SumImpl(a, b, mixA, mixB); }
}
