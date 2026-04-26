#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "TasksCore.h"

#include <atomic>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  namespace
  {
    class ReturningThread : public Neuron::Tasks::Thread
    {
    public:
      explicit ReturningThread(std::atomic_bool& ran)
        : Neuron::Tasks::Thread("ReturningThread"),
          Ran(ran)
      {}

      bool RanOnOwnThread = false;

    protected:
      void* run() override
      {
        RanOnOwnThread = isCurrentThread() && getCurrentThread() == this;
        Ran.store(true);
        return reinterpret_cast<void*>(static_cast<uintptr_t>(0x1234));
      }

    private:
      std::atomic_bool& Ran;
    };

    class WaitingThread : public Neuron::Tasks::Thread
    {
    public:
      explicit WaitingThread(std::atomic_bool& entered)
        : Neuron::Tasks::Thread("WaitingThread"),
          Entered(entered)
      {}

    protected:
      void* run() override
      {
        Entered.store(true);
        while (!stopRequested())
          Sleep(1);

        return nullptr;
      }

    private:
      std::atomic_bool& Entered;
    };
  }

  TEST_CLASS(TasksThreadTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Threading")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(StartupAndShutdownAreCallable)
    {
      Neuron::Tasks::Core::Startup();
      Neuron::Tasks::Core::Shutdown();
    }

    TEST_METHOD(ThreadStartWaitAndReturnValue)
    {
      std::atomic_bool ran(false);
      ReturningThread thread(ran);

      Assert::IsTrue(thread.start());
      Assert::IsTrue(thread.wait());

      void* returnValue = nullptr;
      Assert::IsTrue(thread.getReturnValue(&returnValue));
      Assert::IsTrue(ran.load());
      Assert::IsTrue(thread.RanOnOwnThread);
      Assert::AreEqual(reinterpret_cast<void*>(static_cast<uintptr_t>(0x1234)), returnValue);
      Assert::IsFalse(thread.isRunning());
    }

    TEST_METHOD(ThreadStopRequestsCooperativeExit)
    {
      std::atomic_bool entered(false);
      WaitingThread thread(entered);

      Assert::IsTrue(thread.start());

      for (uint32_t i = 0; i < 1000 && !entered.load(); ++i)
        Sleep(1);

      Assert::IsTrue(entered.load());
      Assert::IsTrue(thread.stop());
      Assert::IsTrue(thread.wait());
      Assert::IsFalse(thread.isRunning());
    }
  };
}
