#include "../Signals.h"

#include "LTE/StdMath.h"

namespace Audio { namespace
  {
    struct Compress : SignalT
    {
      Signal input;
      double factor;

      Compress(const Signal& input, double factor)
        : input(input),
          factor(factor) {}

      double OnGet(const GlobalData& d) override
      {
        double s = input->Get(d);
        return Sign(s) * (1.0 - Exp(-factor * Abs(s)));
      }
    };
  }

  Signal Signal_Compress(const Signal& input, double factor) { return new Compress(input, factor); }
}
