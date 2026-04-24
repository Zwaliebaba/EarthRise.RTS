#include "SoundEngine.h"

#include "LTE/ProgramLog.h"
#include "LTE/Vector.h"

namespace {
  Vector<SoundEngine*> gSoundEngine;
}

SoundEngine::SoundEngine() {
  if (!gSoundEngine.size())
    Push();
}

SoundEngine::~SoundEngine() {
  gSoundEngine.remove(this);
}

void SoundEngine::Pop() {
  LTE_ASSERT(gSoundEngine.size());
  LTE_ASSERT(gSoundEngine.back() == this);
  gSoundEngine.pop();
}

void SoundEngine::Push() {
  gSoundEngine.push(this);
}

SoundEngine* GetSoundEngine() {
  LTE_ASSERT(gSoundEngine.size());
  return gSoundEngine.back();
}

SoundEngine* SoundEngine_Default() {
  return SoundEngine_XAudio2();
}
