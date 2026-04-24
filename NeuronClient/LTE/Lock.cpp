#include "Lock.h"
#include <mutex>

namespace
{
  struct LockImpl : LockT
  {
    std::mutex mutex;

    void Acquire() override { mutex.lock(); }

    void Release() override { mutex.unlock(); }
  };
}

Lock Lock_Create() { return new LockImpl; }
