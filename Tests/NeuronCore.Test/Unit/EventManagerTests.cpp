#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "EventManager.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  namespace
  {
    struct FirstUnitEvent : GameEvent {};
    struct SecondUnitEvent : GameEvent {};
    struct ConsumedUnitEvent : GameEvent {};
    struct DestructorUnitEvent : GameEvent {};

    class MultiEventSubscriber : public EventSubscriber
    {
    public:
      void OnFirst(FirstUnitEvent&) { ++FirstCount; }
      void OnSecond(SecondUnitEvent&) { ++SecondCount; }

      int FirstCount = 0;
      int SecondCount = 0;
    };

    class ConsumingSubscriber : public EventSubscriber
    {
    public:
      void OnEvent(ConsumedUnitEvent& event)
      {
        ++Count;
        event.SetConsumed();
      }

      int Count = 0;
    };

    class CountingConsumedSubscriber : public EventSubscriber
    {
    public:
      void OnEvent(ConsumedUnitEvent&) { ++Count; }

      int Count = 0;
    };

    class ReferencedCounterSubscriber : public EventSubscriber
    {
    public:
      explicit ReferencedCounterSubscriber(int& count)
        : Count(count)
      {}

      void OnEvent(DestructorUnitEvent&) { ++Count; }

    private:
      int& Count;
    };

    int IgnoringProcessorCalls = 0;
    int HandlingProcessorCalls = 0;

    LRESULT CALLBACK IgnoringProcessor(HWND, UINT, WPARAM, LPARAM)
    {
      ++IgnoringProcessorCalls;
      return -1;
    }

    LRESULT CALLBACK HandlingProcessor(HWND, UINT, WPARAM, LPARAM)
    {
      ++HandlingProcessorCalls;
      return 77;
    }
  }

  TEST_CLASS(EventManagerTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Events")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(PublishDispatchesOnlyMatchingEventType)
    {
      MultiEventSubscriber subscriber;
      EventManager::Subscribe<FirstUnitEvent>(&subscriber, &MultiEventSubscriber::OnFirst);
      EventManager::Subscribe<SecondUnitEvent>(&subscriber, &MultiEventSubscriber::OnSecond);

      FirstUnitEvent event;
      EventManager::Publish(event);
      subscriber.UnsubscribeFromAllEvents();

      Assert::AreEqual(1, subscriber.FirstCount);
      Assert::AreEqual(0, subscriber.SecondCount);
    }

    TEST_METHOD(UnsubscribeAllRemovesSubscriberFromEveryEventType)
    {
      MultiEventSubscriber subscriber;
      EventManager::Subscribe<FirstUnitEvent>(&subscriber, &MultiEventSubscriber::OnFirst);
      EventManager::Subscribe<SecondUnitEvent>(&subscriber, &MultiEventSubscriber::OnSecond);

      subscriber.UnsubscribeFromAllEvents();

      FirstUnitEvent firstEvent;
      SecondUnitEvent secondEvent;
      EventManager::Publish(firstEvent);
      EventManager::Publish(secondEvent);

      Assert::AreEqual(0, subscriber.FirstCount);
      Assert::AreEqual(0, subscriber.SecondCount);
    }

    TEST_METHOD(ConsumedEventsStopFurtherDispatch)
    {
      ConsumingSubscriber firstSubscriber;
      CountingConsumedSubscriber secondSubscriber;
      EventManager::Subscribe<ConsumedUnitEvent>(&firstSubscriber, &ConsumingSubscriber::OnEvent);
      EventManager::Subscribe<ConsumedUnitEvent>(&secondSubscriber, &CountingConsumedSubscriber::OnEvent);

      ConsumedUnitEvent event;
      EventManager::Publish(event);

      firstSubscriber.UnsubscribeFromAllEvents();
      secondSubscriber.UnsubscribeFromAllEvents();

      Assert::AreEqual(1, firstSubscriber.Count);
      Assert::AreEqual(0, secondSubscriber.Count);
      Assert::IsTrue(event.IsConsumed());
    }

    TEST_METHOD(SubscriberDestructorUnsubscribesAutomatically)
    {
      int count = 0;
      {
        ReferencedCounterSubscriber subscriber(count);
        EventManager::Subscribe<DestructorUnitEvent>(&subscriber, &ReferencedCounterSubscriber::OnEvent);
      }

      DestructorUnitEvent event;
      EventManager::Publish(event);

      Assert::AreEqual(0, count);
    }

    TEST_METHOD(WindowProcedureProcessorsRunUntilHandled)
    {
      IgnoringProcessorCalls = 0;
      HandlingProcessorCalls = 0;

      EventManager::AddEventProcessor(IgnoringProcessor);
      EventManager::AddEventProcessor(HandlingProcessor);

      LRESULT result = EventManager::WndProc(nullptr, WM_USER, 0, 0);

      EventManager::RemoveEventProcessor(IgnoringProcessor);
      EventManager::RemoveEventProcessor(HandlingProcessor);

      Assert::AreEqual<LRESULT>(77, result);
      Assert::AreEqual(1, IgnoringProcessorCalls);
      Assert::AreEqual(1, HandlingProcessorCalls);
    }
  };
}
