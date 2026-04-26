#include "CppUnitTest.h"

#include "NeuronCore.h"

#include <array>
#include <list>
#include <mutex>
#include <string>

#include "DataReader.h"
#include "DataWriter.h"
#include "EventManager.h"
#include "MathCommon.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  namespace
  {
    struct TestEvent : Event
    {};

    class CountingSubscriber : public EventSubscriber
    {
    public:
      void OnTestEvent(TestEvent& event)
      {
        ++Count;
        if (ConsumeEvent)
          event.SetConsumed();
      }

      int Count = 0;
      bool ConsumeEvent = false;
    };
  }

  TEST_CLASS(NeuronCoreSmokeTests)
  {
  public:
    TEST_METHOD(UmbrellaHeaderCompiles)
    {
      Assert::IsTrue(true);
    }

    TEST_METHOD(Log2HandlesZeroPowersAndNonPowers)
    {
      Assert::AreEqual(0, static_cast<int>(Neuron::Math::Log2(0)));
      Assert::AreEqual(0, static_cast<int>(Neuron::Math::Log2(1)));
      Assert::AreEqual(3, static_cast<int>(Neuron::Math::Log2(8)));
      Assert::AreEqual(4, static_cast<int>(Neuron::Math::Log2(9)));
    }

    TEST_METHOD(AlignmentHelpersRoundToExpectedBoundaries)
    {
      Assert::AreEqual<size_t>(16, Neuron::Math::AlignUp<size_t>(13, 8));
      Assert::AreEqual<size_t>(8, Neuron::Math::AlignDown<size_t>(13, 8));
    }

    TEST_METHOD(DivideByMultipleRoundsUp)
    {
      Assert::AreEqual<size_t>(0, Neuron::Math::DivideByMultiple<size_t>(0, 8));
      Assert::AreEqual<size_t>(1, Neuron::Math::DivideByMultiple<size_t>(1, 8));
      Assert::AreEqual<size_t>(1, Neuron::Math::DivideByMultiple<size_t>(8, 8));
      Assert::AreEqual<size_t>(2, Neuron::Math::DivideByMultiple<size_t>(9, 8));
    }

    TEST_METHOD(PowerAndDivisibilityHelpersReportExpectedValues)
    {
      Assert::IsTrue(Neuron::Math::IsPowerOfTwo<size_t>(8));
      Assert::IsFalse(Neuron::Math::IsPowerOfTwo<size_t>(10));
      Assert::IsTrue(Neuron::Math::IsDivisible<int>(24, 6));
      Assert::IsFalse(Neuron::Math::IsDivisible<int>(25, 6));
    }

    TEST_METHOD(DataWriterAndReaderRoundTripPrimitiveValues)
    {
      Neuron::DataWriter writer;
      writer.Write<uint32_t>(42);
      writer.Write<float>(3.5f);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());

      Assert::AreEqual<uint32_t>(42, reader.Read<uint32_t>());
      Assert::AreEqual(3.5f, reader.Read<float>(), 0.0001f);
    }

    TEST_METHOD(DataWriterAndReaderRoundTripStrings)
    {
      Neuron::DataWriter writer;
      writer.WriteString("EarthRise");

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());

      Assert::AreEqual(std::string("EarthRise"), reader.ReadString());
    }

    TEST_METHOD(DataReaderReadsTriviallyCopyableArrays)
    {
      Neuron::DataWriter writer;
      writer.Write<int32_t>(3);
      writer.Write<int32_t>(5);
      writer.Write<int32_t>(8);

      std::array<int32_t, 3> values{};
      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      reader.ReadArray(values.data(), values.size());

      Assert::AreEqual<int32_t>(3, values[0]);
      Assert::AreEqual<int32_t>(5, values[1]);
      Assert::AreEqual<int32_t>(8, values[2]);
    }

    TEST_METHOD(AssertReleaseAllowsTrueExpressions)
    {
      ASSERT_RELEASE(true);
      ASSERT_RELEASE_TEXT(true, "release assertion should not fail");
    }

    TEST_METHOD(AssertReleaseTextThrowsWithMessageOnFailure)
    {
      try
      {
        ASSERT_RELEASE_TEXT(false, "release assertion {}", 7);
        Assert::Fail(L"ASSERT_RELEASE_TEXT should throw on failure");
      }
      catch (const std::runtime_error& Fatal)
      {
        Assert::AreEqual("release assertion 7", Fatal.what());
      }
    }

    TEST_METHOD(EventManagerPublishesToSubscribers)
    {
      CountingSubscriber subscriber;
      EventManager::Subscribe<TestEvent>(&subscriber, &CountingSubscriber::OnTestEvent);

      TestEvent event;
      EventManager::Publish(event);

      Assert::AreEqual(1, subscriber.Count);
    }

    TEST_METHOD(EventManagerStopsDispatchWhenEventIsConsumed)
    {
      CountingSubscriber firstSubscriber;
      CountingSubscriber secondSubscriber;
      firstSubscriber.ConsumeEvent = true;

      EventManager::Subscribe<TestEvent>(&firstSubscriber, &CountingSubscriber::OnTestEvent);
      EventManager::Subscribe<TestEvent>(&secondSubscriber, &CountingSubscriber::OnTestEvent);

      TestEvent event;
      EventManager::Publish(event);

      firstSubscriber.UnsubscribeFromAllEvents();
      secondSubscriber.UnsubscribeFromAllEvents();

      Assert::AreEqual(1, firstSubscriber.Count);
      Assert::AreEqual(0, secondSubscriber.Count);
      Assert::IsTrue(event.IsConsumed());
    }
  };
}
