#include "CppUnitTest.h"

#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Integration
{
  TEST_CLASS(NeuronCoreIntegrationTests)
  {
  public:
    TEST_METHOD(SourceDirectoryExists)
    {
      auto const root = std::filesystem::path(EARTHRISE_TEST_SOURCE_DIR);
      Assert::IsTrue(std::filesystem::exists(root / "NeuronCore"));
    }
  };
}
