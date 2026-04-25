#include "Thread.h"

#include "Job.h"
#include "Lock.h"

#include "SFML/System.hpp"

#include <Windows.h>
#include <memory>
#include <thread>

#undef GetJob

namespace
{
  Lock GetThreadLock()
  {
    /* NOTE : Locks cannot be static destructed. */
    static auto lock = new Lock(Lock_Create());
    return *lock;
  }

  struct ThreadImpl : ThreadT
  {
    Job job;
    std::unique_ptr<std::thread> thread;
    bool finished;

    ThreadImpl(const Job& job)
      : job(job),
        finished(false)
    {
      ScopedLock lock(GetThreadLock());
      job->OnBegin();
      thread.reset(new std::thread(&ThreadImpl::Run, this));
    }

    ~ThreadImpl() override
    {
      if (thread && thread->joinable()) {
        if (finished)
          thread->join();
        else
          Terminate();
      }
      job->OnEnd();
    }

    Job GetJob() const override { return job; }

    bool IsFinished() const override { return finished; }

    void Run()
    {
      job->OnRun(UINT_MAX);
      finished = true;
    }

    void Terminate() override
    {
      ScopedLock lock(GetThreadLock());
      if (thread && thread->joinable()) {
        ::TerminateThread((HANDLE)thread->native_handle(), 0);
        thread->detach();
      }
      finished = true;
    }

    void Wait() override {
      if (thread && thread->joinable())
        thread->join();
    }
  };
}

Thread Thread_Create(const Job& job) { return new ThreadImpl(job); }

DefineFunction(Thread_SleepMS) { sf::sleep(sf::milliseconds(args.ms)); }

DefineFunction(Thread_SleepUS) { sf::sleep(sf::microseconds(args.us)); }
