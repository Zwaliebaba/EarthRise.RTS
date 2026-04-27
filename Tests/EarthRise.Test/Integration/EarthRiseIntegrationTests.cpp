#include "CppUnitTest.h"

#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EarthRiseTest::Integration
{
  TEST_CLASS(EarthRiseIntegrationTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Integration.AssetStaging")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(GameDataDirectoryExists)
    {
      auto const root = std::filesystem::path(EARTHRISE_TEST_SOURCE_DIR);
      Assert::IsTrue(std::filesystem::exists(root / "GameData"));
    }
  };
}
