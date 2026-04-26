#ifndef Audio_Signal_h__
#define Audio_Signal_h__

#include "LTE/Reference.h"

namespace Audio
{
  using Generator = Reference<GeneratorT>;

  struct GlobalData
  {
    uint sampleNum;
    uint sampleRate;
  };

  struct SignalT : RefCounted
  {
    struct SignalImpl* impl;

    SignalT();
    ~SignalT() override;

    virtual double OnGet(const GlobalData& d) = 0;

    double Get(const GlobalData& d);
  };
  using Signal = Reference<SignalT>;

  Array<float>* Signal_Render(const Signal& signal, uint seconds, uint bpm = 120, uint rate = 44100);
}

#endif
