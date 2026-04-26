#include "Thread.h"

#include "Job.h"
#include "Lock.h"

#include "SFML/System.hpp"

#include <Windows.h>
#include <atomic>
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
    /* Job ownership is handed to the worker; shared mutation remains external. */
    Job job;
    std::unique_ptr<std::thread> thread;
    std::atomic_bool finished;

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
        if (finished.load())
          thread->join();
        else
          Terminate();
      }
      job->OnEnd();
    }

    Job GetJob() const override { return job; }

    bool IsFinished() const override { return finished.load(); }

    void Run()
    {
      job->OnRun(UINT_MAX);
      finished.store(true);
    }

    void Terminate() override
    {
      ScopedLock lock(GetThreadLock());
      if (thread && thread->joinable()) {
        ::TerminateThread((HANDLE)thread->native_handle(), 0);
        thread->detach();
      }
      finished.store(true);
    }

    void Wait() override {
      if (thread && thread->joinable()) {
        thread->join();
        finished.store(true);
      }
    }
  };
}

Thread Thread_Create(const Job& job) { return new ThreadImpl(job); }

DefineFunction(Thread_SleepMS) { sf::sleep(sf::milliseconds(args.ms)); }

DefineFunction(Thread_SleepUS) { sf::sleep(sf::microseconds(args.us)); }
