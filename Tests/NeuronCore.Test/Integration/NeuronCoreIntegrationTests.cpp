#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "TimerCore.h"

#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Integration
{
  TEST_CLASS(NeuronCoreIntegrationTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Integration.CoreRuntime")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(SourceDirectoryExists)
    {
      auto const root = std::filesystem::path(EARTHRISE_TEST_SOURCE_DIR);
      Assert::IsTrue(std::filesystem::exists(root / "NeuronCore"));
    }

    TEST_METHOD(CoreEngineStartupInitializesTimer)
    {
      bool shutdownNeeded = false;
      uint32_t frameBefore = 0;
      uint32_t frameAfter = 0;
      float elapsedSeconds = 0.0f;

      try
      {
        Neuron::CoreEngine::Startup();
        shutdownNeeded = true;

        frameBefore = Neuron::Timer::Core::GetFrameCount();
        Sleep(1);
        elapsedSeconds = Neuron::Timer::Core::Update();
        frameAfter = Neuron::Timer::Core::GetFrameCount();

        Neuron::CoreEngine::Shutdown();
        shutdownNeeded = false;
      }
      catch (...)
      {
        if (shutdownNeeded)
          Neuron::CoreEngine::Shutdown();
        throw;
      }

      Assert::AreEqual<uint32_t>(frameBefore + 1, frameAfter);
      Assert::IsTrue(elapsedSeconds >= 0.0f);
      Assert::IsTrue(Neuron::Timer::Core::GetTotalTicks() >= Neuron::Timer::Core::GetElapsedTicks());
    }
  };
}
