#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "GameMath.h"
#include "MathCommon.h"
#include "TimerCore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  TEST_CLASS(MathAndTimerTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.MathAndTimer")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(Log2RoundsUpForNonPowersOfTwo)
    {
      Assert::AreEqual(0, static_cast<int>(Neuron::Math::Log2(0)));
      Assert::AreEqual(0, static_cast<int>(Neuron::Math::Log2(1)));
      Assert::AreEqual(1, static_cast<int>(Neuron::Math::Log2(2)));
      Assert::AreEqual(2, static_cast<int>(Neuron::Math::Log2(3)));
      Assert::AreEqual(8, static_cast<int>(Neuron::Math::Log2(255)));
      Assert::AreEqual(8, static_cast<int>(Neuron::Math::Log2(256)));
    }

    TEST_METHOD(AlignmentHelpersRoundIntegralValues)
    {
      Assert::AreEqual<size_t>(16, Neuron::Math::AlignUp<size_t>(13, 8));
      Assert::AreEqual<size_t>(8, Neuron::Math::AlignDown<size_t>(13, 8));
      Assert::AreEqual<size_t>(32, Neuron::Math::AlignUpWithMask<size_t>(17, 15));
      Assert::AreEqual<size_t>(16, Neuron::Math::AlignDownWithMask<size_t>(31, 15));
    }

    TEST_METHOD(AlignmentHelpersRoundPointers)
    {
      std::array<std::byte, 64> storage = {};
      void* unaligned = storage.data() + 3;
      void* alignedUp = Neuron::Math::AlignUp<void*>(unaligned, 16);
      void* alignedDown = Neuron::Math::AlignDown<void*>(unaligned, 16);

      Assert::IsTrue(reinterpret_cast<uintptr_t>(alignedUp) % 16 == 0);
      Assert::IsTrue(reinterpret_cast<uintptr_t>(alignedDown) % 16 == 0);
      Assert::IsTrue(reinterpret_cast<uintptr_t>(alignedUp) >= reinterpret_cast<uintptr_t>(unaligned));
      Assert::IsTrue(reinterpret_cast<uintptr_t>(alignedDown) <= reinterpret_cast<uintptr_t>(unaligned));
      Assert::IsTrue(Neuron::Math::IsAligned(alignedUp, 16));
    }

    TEST_METHOD(DivisionAndDivisibilityHelpersReportExpectedValues)
    {
      Assert::AreEqual<size_t>(0, Neuron::Math::DivideByMultiple<size_t>(0, 8));
      Assert::AreEqual<size_t>(1, Neuron::Math::DivideByMultiple<size_t>(1, 8));
      Assert::AreEqual<size_t>(1, Neuron::Math::DivideByMultiple<size_t>(8, 8));
      Assert::AreEqual<size_t>(2, Neuron::Math::DivideByMultiple<size_t>(9, 8));
      Assert::IsTrue(Neuron::Math::IsPowerOfTwo<size_t>(8));
      Assert::IsFalse(Neuron::Math::IsPowerOfTwo<size_t>(10));
      Assert::IsTrue(Neuron::Math::IsDivisible<int>(24, 6));
      Assert::IsFalse(Neuron::Math::IsDivisible<int>(25, 6));
    }

    TEST_METHOD(VectorHelpersComputeExpectedResults)
    {
      auto const unitX = Neuron::Math::Set(1.0f, 0.0f, 0.0f);
      auto const unitY = Neuron::Math::Set(0.0f, 1.0f, 0.0f);
      auto const cross = Neuron::Math::Cross(unitX, unitY);
      auto const scaled = Neuron::Math::SetLength(Neuron::Math::Set(3.0f, 0.0f, 0.0f), 5.0f);
      auto const rotated = Neuron::Math::RotateAroundZ(unitX, DirectX::XM_PIDIV2);

      Assert::AreEqual(0.0f, Neuron::Math::Dotf(unitX, unitY), 0.0001f);
      Assert::AreEqual(1.0f, Neuron::Math::GetZ(cross), 0.0001f);
      Assert::AreEqual(5.0f, Neuron::Math::Length(scaled), 0.0001f);
      Assert::AreEqual(0.0f, Neuron::Math::GetX(rotated), 0.0001f);
      Assert::AreEqual(1.0f, Neuron::Math::GetY(rotated), 0.0001f);
    }

    TEST_METHOD(TimerTickConversionsAreStable)
    {
      Assert::AreEqual(0.0, Neuron::Timer::Core::TicksToSeconds(0), 0.000001);
      Assert::AreEqual(1.25, Neuron::Timer::Core::TicksToSeconds(12500000), 0.000001);
      Assert::AreEqual<uint64_t>(uint64_t{10000000}, Neuron::Timer::Core::SecondsToTicks(1.0));
      Assert::AreEqual<uint64_t>(uint64_t{12500000}, Neuron::Timer::Core::SecondsToTicks(1.25));
    }
  };
}
