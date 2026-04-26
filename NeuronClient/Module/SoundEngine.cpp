#include "SoundEngine.h"
#include "LTE/Vector.h"

namespace
{
  Vector<SoundEngine*> gSoundEngine;
}

SoundEngine::SoundEngine()
{
  if (!gSoundEngine.size())
    Push();
}

SoundEngine::~SoundEngine() { gSoundEngine.remove(this); }

void SoundEngine::Pop()
{
  DEBUG_ASSERT(gSoundEngine.size());
  DEBUG_ASSERT(gSoundEngine.back() == this);
  gSoundEngine.pop();
}

void SoundEngine::Push() { gSoundEngine.push(this); }

SoundEngine* GetSoundEngine()
{
  DEBUG_ASSERT(gSoundEngine.size());
  return gSoundEngine.back();
}

SoundEngine* SoundEngine_Default() { return SoundEngine_XAudio2(); }
