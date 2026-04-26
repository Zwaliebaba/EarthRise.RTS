#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "DataReader.h"
#include "DataWriter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Unit
{
  TEST_CLASS(DataSerializationTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Unit.Serialization")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(NewWriterStartsEmpty)
    {
      Neuron::DataWriter writer;

      Assert::AreEqual(0, writer.Size());
      Assert::IsNotNull(writer.Data());
    }

    TEST_METHOD(WriterClearResetsSizeWithoutInvalidatingBuffer)
    {
      Neuron::DataWriter writer;
      char const* originalBuffer = writer.Data();

      writer.Write<uint32_t>(42);
      writer.Clear();

      Assert::AreEqual(0, writer.Size());
      Assert::AreEqual(originalBuffer, writer.Data());
    }

    TEST_METHOD(PrimitiveRoundTripPreservesValuesAndOrder)
    {
      Neuron::DataWriter writer;
      writer.Write<uint8_t>(0x7f);
      writer.Write<int16_t>(-1234);
      writer.Write<uint32_t>(0xfeedbeef);
      writer.Write<float>(12.5f);

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());

      Assert::AreEqual(0x7f, static_cast<int>(reader.Read<uint8_t>()));
      Assert::AreEqual<int16_t>(-1234, reader.Read<int16_t>());
      Assert::AreEqual<uint32_t>(0xfeedbeef, reader.Read<uint32_t>());
      Assert::AreEqual(12.5f, reader.Read<float>(), 0.0001f);
    }

    TEST_METHOD(StringRoundTripHandlesEmptyAndNonEmptyStrings)
    {
      Neuron::DataWriter writer;
      writer.WriteString("");
      writer.WriteString("EarthRise");

      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());

      Assert::IsTrue(reader.ReadString().empty());
      Assert::IsTrue(reader.ReadString() == "EarthRise");
    }

    TEST_METHOD(ArrayRoundTripReadsContiguousTriviallyCopyableValues)
    {
      std::array<int32_t, 4> expected = {1, 1, 2, 3};
      Neuron::DataWriter writer;

      for (int32_t value : expected)
        writer.Write<int32_t>(value);

      std::array<int32_t, 4> actual = {};
      Neuron::DataReader reader(reinterpret_cast<const uint8_t*>(writer.Data()), writer.Size());
      reader.ReadArray(actual.data(), actual.size());

      Assert::IsTrue(actual == expected);
    }

    TEST_METHOD(WriterRejectsOversizedStrings)
    {
      Neuron::DataWriter writer;
      std::string oversized(1500, 'x');

      Assert::ExpectException<std::runtime_error>([&writer, &oversized]
      {
        writer.WriteString(oversized);
      });
    }

    TEST_METHOD(ReaderRejectsOversizedInputBuffers)
    {
      std::array<uint8_t, 1500> oversized = {};

      Assert::ExpectException<std::runtime_error>([&oversized]
      {
        Neuron::DataReader reader(oversized.data(), oversized.size());
      });
    }
  };
}
