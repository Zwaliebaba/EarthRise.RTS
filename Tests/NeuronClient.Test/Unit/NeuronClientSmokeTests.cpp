#include "CppUnitTest.h"

#include "NeuronClient.h"

#include "LTE/Array.h"
#include "LTE/Job.h"
#include "LTE/Thread.h"
#include "LTE/Tuple.h"

#include <atomic>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronClientTest::Unit
{
  struct ThreadSmokeJob : LTE::JobT
  {
    std::atomic_bool& ran;

    ThreadSmokeJob(std::atomic_bool& ran)
      : ran(ran)
    {}

    char const* GetName() const override { return "ThreadSmokeJob"; }

    void OnRun(uint) override
    {
      ran.store(true);
    }
  };

  TEST_CLASS(NeuronClientSmokeTests)
  {
  public:
    TEST_METHOD(UmbrellaHeaderCompiles)
    {
      Assert::IsTrue(true);
    }

    TEST_METHOD(Tuple3EqualityComparesThirdElement)
    {
      auto const left = Tuple(1, 2, 3);
      auto const right = Tuple(1, 2, 4);

      Assert::IsFalse(left == right);
      Assert::IsTrue(left != right);
    }

    TEST_METHOD(Tuple4EqualityComparesFourthElement)
    {
      auto const left = Tuple(1, 2, 3, 4);
      auto const right = Tuple(1, 2, 3, 5);

      Assert::IsFalse(left == right);
      Assert::IsTrue(left != right);
    }

    TEST_METHOD(ArrayEqualityComparesEveryElement)
    {
      Array<uint32_t> left(2);
      Array<uint32_t> right(2);
      left[0] = 1;
      left[1] = 0;
      right[0] = 1;
      right[1] = 1;

      Assert::IsFalse(left == right);
    }

    TEST_METHOD(ArrayEqualityAcceptsMatchingValues)
    {
      Array<uint32_t> left(3);
      Array<uint32_t> right(3);
      left[0] = 1;
      left[1] = 2;
      left[2] = 3;
      right[0] = 1;
      right[1] = 2;
      right[2] = 3;

      Assert::IsTrue(left == right);
    }

    TEST_METHOD(ThreadWaitMarksJobFinished)
    {
      std::atomic_bool ran(false);
      Thread thread = Thread_Create(new ThreadSmokeJob(ran));

      thread->Wait();

      Assert::IsTrue(ran.load());
      Assert::IsTrue(thread->IsFinished());
    }
  };
}
