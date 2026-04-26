#ifndef Audio_Signals_h__
#define Audio_Signals_h__

#include "Signal.h"
#include "Note.h"

namespace Audio
{
  Signal Signal_ASR(double a, double s, double r);

  Signal Signal_Compress(const Signal& input, double factor);

  Signal Signal_Delay(const Signal& input, uint ticks, double amp, double feedback);

  Signal Signal_Instrument(const Generator& generator, const Pattern& pattern, const Signal& envelope = nullptr);

  Signal Signal_Lowpass(const Signal& input);

  Signal Signal_Product(const Signal& a, const Signal& b);

  Signal Signal_Sum(const Signal& a, const Signal& b, double mixA = 1, double mixB = 1);
}

#endif
