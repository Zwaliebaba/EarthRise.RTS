#ifndef Audio_Note_h__
#define Audio_Note_h__

#include "LTE/Vector.h"

namespace Audio
{
  struct Note
  {
    uint m_on;
    uint off;
    double frequency;
    uint duration;
    float velocity;

    Note() {}

    Note(uint time, double frequency, uint duration, float velocity = 1)
      : m_on(time),
        off(time + duration),
        frequency(frequency),
        velocity(velocity) {}

    friend bool operator<(const Note& a, const Note& b) { return a.m_on < b.m_on; }
  };

  using Pattern = Vector<Note>;
}

#endif
