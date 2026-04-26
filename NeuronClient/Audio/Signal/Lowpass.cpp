#include "../Signals.h"

#include "LTE/StdMath.h"

namespace Audio { namespace
  {
    struct Lowpass : SignalT
    {
      Signal input;
      double s;

      Lowpass(const Signal& input)
        : input(input),
          s(0) {}

      double OnGet(const GlobalData& d) override { return s = Mix(s, input->Get(d), Exp(-(d.sampleRate / 5000.0))); }
    };
  }

  Signal Signal_Lowpass(const Signal& input) { return new Lowpass(input); }
}
