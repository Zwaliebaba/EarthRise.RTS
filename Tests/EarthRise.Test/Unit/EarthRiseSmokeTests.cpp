#include "CppUnitTest.h"

#include "EarthRise.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EarthRiseTest::Unit
{
  TEST_CLASS(EarthRiseSmokeTests)
  {
  public:
    TEST_METHOD(UmbrellaHeaderCompiles)
    {
      Assert::IsTrue(true);
    }
  };
}
