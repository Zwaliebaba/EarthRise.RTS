#ifndef Audio_IO_h__
#define Audio_IO_h__

#include "Common.h"

namespace Audio
{
  void WAV_Write(const char* outFile, const Array<int>& buf, int sampleRate, short channels);
}

#endif
