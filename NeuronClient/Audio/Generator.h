#ifndef Audio_Generator_h__
#define Audio_Generator_h__

#include "Note.h"
#include "LTE/Reference.h"

namespace Audio
{
  struct GeneratorT : RefCounted
  {
    ~GeneratorT() override {}
    virtual double Get(const Note& note, double t) const = 0;
  };

  using Generator = Reference<GeneratorT>;
}

#endif
