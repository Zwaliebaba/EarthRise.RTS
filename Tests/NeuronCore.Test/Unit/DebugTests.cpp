#include "CppUnitTest.h"

#include "NeuronCore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  TEST_CLASS(DebugTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Diagnostics")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(ReleaseAssertionsAllowTrueExpressions)
    {
      ASSERT_RELEASE(true);
      ASSERT_RELEASE_TEXT(true, "release assertion should not fail");
    }

    TEST_METHOD(ReleaseAssertionThrowsOnFailure)
    {
      Assert::ExpectException<std::runtime_error>([]
      {
        ASSERT_RELEASE(false);
      });
    }

    TEST_METHOD(FatalFormatsNarrowMessages)
    {
      try
      {
        Neuron::Fatal("fatal {}", 17);
        Assert::Fail(L"Fatal should throw");
      }
      catch (const std::runtime_error& error)
      {
        Assert::IsTrue(std::string(error.what()) == "fatal 17");
      }
    }

    TEST_METHOD(FatalWideMessagesThrowRuntimeError)
    {
      try
      {
        Neuron::Fatal(L"fatal {}", 23);
        Assert::Fail(L"Fatal should throw");
      }
      catch (const std::runtime_error& error)
      {
        Assert::IsTrue(std::string(error.what()) == "Fatal Error");
      }
    }
  };
}
