#include "SoundEngine.h"

#include <algorithm>
#include <vector>

namespace
{
  std::vector<SoundEngine*> gSoundEngine;
}

SoundEngine::SoundEngine()
{
  if (!gSoundEngine.size())
    Push();
}

SoundEngine::~SoundEngine()
{
  auto it = std::find(gSoundEngine.begin(), gSoundEngine.end(), this);
  if (it != gSoundEngine.end())
    gSoundEngine.erase(it);
}

void SoundEngine::Pop()
{
  DEBUG_ASSERT(gSoundEngine.size());
  DEBUG_ASSERT(gSoundEngine.back() == this);
  gSoundEngine.pop_back();
}

void SoundEngine::Push() { gSoundEngine.push_back(this); }

SoundEngine* GetSoundEngine()
{
  DEBUG_ASSERT(gSoundEngine.size());
  return gSoundEngine.back();
}

SoundEngine* SoundEngine_Default() { return SoundEngine_XAudio2(); }
