#include "../Signals.h"

#include "LTE/Array.h"
#include "LTE/StdMath.h"

namespace Audio { namespace
  {
    struct Delay : SignalT
    {
      Signal input;
      Array<double> buffer;
      double amp;
      double feedback;
      uint offset;

      Delay(const Signal& input, uint samples, double amp, double feedback)
        : input(input),
          buffer(samples),
          amp(amp),
          feedback(feedback),
          offset(0) {}

      double OnGet(const GlobalData& d) override
      {
        double s = input->Get(d);
        if (d.sampleNum >= buffer.size())
          s += (amp / feedback) * buffer[offset];
        buffer[offset++] = feedback * s;
        offset %= buffer.size();
        return s;
      }
    };
  }

  Signal Signal_Delay(const Signal& input, uint samples, double amp, double feedback) { return new Delay(input, samples, amp, feedback); }
}
