#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "ASyncLoader.h"

#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  namespace
  {
    class TestAsyncLoader : public ASyncLoader
    {
    public:
      void BeginLoad() { StartLoading(); }
      void CompleteLoad() { FinishLoading(); }
    };

    class TestStaticASyncLoader : public StaticASyncLoader
    {
    public:
      static void BeginLoad() { StartLoading(); }
      static void CompleteLoad() { FinishLoading(); }
    };
  }

  TEST_CLASS(AsyncLoaderTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.AsyncLoader")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(InstanceLoaderTracksLoadingAndValidity)
    {
      TestAsyncLoader loader;

      Assert::IsFalse(loader.IsValid());
      Assert::IsFalse(loader.IsLoading());

      loader.BeginLoad();

      Assert::IsTrue(loader.IsLoading());
      Assert::IsFalse(loader.IsValid());

      loader.CompleteLoad();

      Assert::IsFalse(loader.IsLoading());
      Assert::IsTrue(loader.IsValid());
    }

    TEST_METHOD(WaitForLoadReturnsAfterCompletion)
    {
      TestAsyncLoader loader;
      loader.BeginLoad();

      std::thread finisher([&loader]
      {
        loader.CompleteLoad();
      });

      loader.WaitForLoad();
      finisher.join();

      Assert::IsFalse(loader.IsLoading());
      Assert::IsTrue(loader.IsValid());
    }

    TEST_METHOD(StaticLoaderTracksLoadingAndValidity)
    {
      Assert::IsFalse(TestStaticASyncLoader::IsValid());

      TestStaticASyncLoader::BeginLoad();
      TestStaticASyncLoader::CompleteLoad();
      TestStaticASyncLoader::WaitForLoad();

      Assert::IsTrue(TestStaticASyncLoader::IsValid());
    }
  };
}
