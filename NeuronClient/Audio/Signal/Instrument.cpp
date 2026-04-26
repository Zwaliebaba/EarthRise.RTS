#include "../Signals.h"

#include "Audio/Generator.h"

#include "LTE/AutoPtr.h"
#include "LTE/StdMath.h"

namespace Audio { namespace
  {
    struct Instrument : SignalT
    {
      Generator generator;
      Signal envelope;
      Pattern pattern;
      size_t offset;

      Instrument(const Generator& generator, const Pattern& pattern, const Signal& envelope)
        : generator(generator),
          envelope(envelope),
          pattern(pattern) {}

      double OnGet(const GlobalData& d) override
      {
        double sum = 0;
        for (size_t i = 0; i < pattern.size(); ++i)
        {
          const Note& note = pattern[i];
          if (note.m_on <= d.sampleNum && d.sampleNum <= note.off)
          {
            uint tick = d.sampleNum - note.m_on;
            double lt = static_cast<double>(tick) / d.sampleRate;
            double signal = generator->Get(note, lt);
            if (envelope)
            {
              GlobalData ld = {tick, d.sampleRate};
              signal *= envelope->Get(ld);
            }
            sum += signal;
          }
        }
        return sum;
      }
    };
  }

  Signal Signal_Instrument(const Generator& generator, const Pattern& pattern, const Signal& envelope)
  {
    return new Instrument(generator, pattern, envelope);
  }
}
