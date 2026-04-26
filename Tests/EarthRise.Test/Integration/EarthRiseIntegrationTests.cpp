#include "CppUnitTest.h"

#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EarthRiseTest::Integration
{
  TEST_CLASS(EarthRiseIntegrationTests)
  {
  public:
    TEST_METHOD(GameDataDirectoryExists)
    {
      auto const root = std::filesystem::path(EARTHRISE_TEST_SOURCE_DIR);
      Assert::IsTrue(std::filesystem::exists(root / "GameData"));
    }
  };
}
