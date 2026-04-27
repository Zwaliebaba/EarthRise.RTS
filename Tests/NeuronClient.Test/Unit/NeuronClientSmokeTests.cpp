#include "CppUnitTest.h"

#include "NeuronClient.h"

#include "LTE/Array.h"
#include "LTE/Job.h"
#include "LTE/Reference.h"
#include "LTE/Thread.h"
#include "LTE/Tuple.h"
#include "LTE/Type.h"

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

  struct ThreadCancelJob : LTE::JobT
  {
    std::atomic_bool& started;
    std::atomic_bool& cancelRequested;
    std::atomic_bool& exited;

    ThreadCancelJob(
      std::atomic_bool& started,
      std::atomic_bool& cancelRequested,
      std::atomic_bool& exited)
      : started(started),
        cancelRequested(cancelRequested),
        exited(exited)
    {}

    char const* GetName() const override { return "ThreadCancelJob"; }

    void OnCancel() override
    {
      cancelRequested.store(true);
    }

    void OnRun(uint) override
    {
      started.store(true);
      while (!cancelRequested.load())
        Thread_SleepMS(1);
      exited.store(true);
    }
  };

  struct MetadataLookupJob : LTE::JobT
  {
    Type expected;
    std::atomic_bool& ran;
    std::atomic_bool& foundExpected;

    MetadataLookupJob(
      Type const& expected,
      std::atomic_bool& ran,
      std::atomic_bool& foundExpected)
      : expected(expected),
        ran(ran),
        foundExpected(foundExpected)
    {}

    char const* GetName() const override { return "MetadataLookupJob"; }

    void OnRun(uint) override
    {
      foundExpected.store(Type_Find("unsigned int") == expected);
      ran.store(true);
    }
  };

  struct CountingRef : RefCounted
  {
    std::atomic_uint& destroyed;

    explicit CountingRef(std::atomic_uint& destroyed)
      : destroyed(destroyed)
    {}

    ~CountingRef() override
    {
      destroyed.fetch_add(1);
    }
  };

  TEST_CLASS(NeuronClientSmokeTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Smoke")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(UmbrellaHeaderCompiles)
    {
      Assert::IsTrue(true);
    }
  };

  TEST_CLASS(NeuronClientTypeTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Types")
    END_TEST_CLASS_ATTRIBUTE()

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

    TEST_METHOD(ReferenceTracksSingleThreadCopies)
    {
      std::atomic_uint destroyed(0);
      Reference<CountingRef> owner(new CountingRef(destroyed));
      CountingRef* raw = owner.get();

      Assert::AreEqual(1u, raw->refCount);

      {
        Reference<CountingRef> copy = owner;
        Assert::AreEqual(2u, raw->refCount);
      }

      Assert::AreEqual(1u, raw->refCount);
      Assert::AreEqual(0u, destroyed.load());

      owner.reset();

      Assert::AreEqual(1u, destroyed.load());
    }

    TEST_METHOD(TypeGetReturnsStableStartupMetadata)
    {
      Type first = Type_Get<unsigned int>();
      Type second = Type_Get<unsigned int>();

      Assert::IsTrue(first == second);
      Assert::IsTrue(Type_Find("unsigned int") == first);
      Assert::AreEqual(sizeof(unsigned int), first->size);
    }

    TEST_METHOD(TypeFindReturnsNullForUnknownName)
    {
      Type missing = Type_Find("__EarthRiseMissingTypeForPhase3Test__");

      Assert::IsFalse(missing);
    }
  };

  TEST_CLASS(NeuronClientThreadTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Threading")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(ThreadWaitMarksJobFinished)
    {
      std::atomic_bool ran(false);
      Thread thread = Thread_Create(new ThreadSmokeJob(ran));

      thread->Wait();

      Assert::IsTrue(ran.load());
      Assert::IsTrue(thread->IsFinished());
    }

    TEST_METHOD(ThreadTerminateRequestsCooperativeStop)
    {
      std::atomic_bool started(false);
      std::atomic_bool cancelRequested(false);
      std::atomic_bool exited(false);
      Thread thread = Thread_Create(new ThreadCancelJob(started, cancelRequested, exited));

      for (uint i = 0; i < 1000 && !started.load(); ++i)
        Thread_SleepMS(1);

      Assert::IsTrue(started.load());
      thread->Terminate();

      Assert::IsTrue(cancelRequested.load());
      Assert::IsTrue(exited.load());
      Assert::IsTrue(thread->IsFinished());
    }

    TEST_METHOD(ThreadReleaseRequestsCooperativeStop)
    {
      std::atomic_bool started(false);
      std::atomic_bool cancelRequested(false);
      std::atomic_bool exited(false);

      {
        Thread thread = Thread_Create(new ThreadCancelJob(started, cancelRequested, exited));

        for (uint i = 0; i < 1000 && !started.load(); ++i)
          Thread_SleepMS(1);

        Assert::IsTrue(started.load());
      }

      Assert::IsTrue(cancelRequested.load());
      Assert::IsTrue(exited.load());
    }

    TEST_METHOD(ThreadCanReadPreRegisteredMetadata)
    {
      Type expected = Type_Get<unsigned int>();
      std::atomic_bool ran(false);
      std::atomic_bool foundExpected(false);
      Thread thread = Thread_Create(new MetadataLookupJob(expected, ran, foundExpected));

      thread->Wait();

      Assert::IsTrue(ran.load());
      Assert::IsTrue(foundExpected.load());
      Assert::IsTrue(thread->IsFinished());
    }
  };
}
