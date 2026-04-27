#include "CppUnitTest.h"

#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronClientTest::Integration
{
  TEST_CLASS(NeuronClientIntegrationTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Integration.ClientRuntime")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(SourceDirectoryExists)
    {
      auto const root = std::filesystem::path(EARTHRISE_TEST_SOURCE_DIR);
      Assert::IsTrue(std::filesystem::exists(root / "NeuronClient"));
    }
  };
}
