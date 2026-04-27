#include "CppUnitTest.h"

#include "EarthRise.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EarthRiseTest::Unit
{
  TEST_CLASS(EarthRiseSmokeTests)
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
}
