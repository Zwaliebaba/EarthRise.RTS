#include "CppUnitTest.h"

#include "NeuronCore.h"
#include "FileSys.h"

#include <filesystem>
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NeuronCoreTest::Integration
{
  namespace
  {
    std::filesystem::path MakeUniqueTestRoot()
    {
      return std::filesystem::temp_directory_path()
        / L"EarthRise.NeuronCore.Test"
        / std::to_wstring(GetCurrentProcessId())
        / std::to_wstring(GetTickCount64());
    }

    void RestoreHomeDirectory(const std::wstring& previousHome)
    {
      if (previousHome.empty())
        return;

      std::wstring previousRoot = previousHome;
      constexpr std::wstring_view suffix = L"\\GameData\\";
      if (previousRoot.ends_with(suffix))
        previousRoot.resize(previousRoot.size() - suffix.size());

      Neuron::FileSys::SetHomeDirectory(previousRoot);
    }
  }

  TEST_CLASS(FileSysIntegrationTests)
  {
  public:
    BEGIN_TEST_CLASS_ATTRIBUTE()
      TEST_CLASS_ATTRIBUTE(L"Category", L"Integration.FileSystem")
    END_TEST_CLASS_ATTRIBUTE()

    TEST_METHOD(HomeDirectoryAppendsGameDataFolder)
    {
      std::wstring previousHome = Neuron::FileSys::GetHomeDirectory();
      auto const root = MakeUniqueTestRoot();

      Neuron::FileSys::SetHomeDirectory(root.wstring());

      Assert::IsTrue(Neuron::FileSys::GetHomeDirectory() == root.wstring() + L"\\GameData\\");
      Assert::IsTrue(Neuron::FileSys::GetHomeDirectoryA().ends_with("\\GameData\\"));

      RestoreHomeDirectory(previousHome);
    }

    TEST_METHOD(BinaryFileWritesAndReadsFromHomeGameData)
    {
      std::wstring previousHome = Neuron::FileSys::GetHomeDirectory();
      auto const root = MakeUniqueTestRoot();
      std::filesystem::create_directories(root / L"GameData");
      Neuron::FileSys::SetHomeDirectory(root.wstring());

      Neuron::byte_buffer_t expected = {0, 1, 2, 3, 254, 255};

      Assert::IsTrue(Neuron::BinaryFile::WriteFile(L"binary.dat", expected));
      Neuron::byte_buffer_t actual = Neuron::BinaryFile::ReadFile(L"binary.dat");

      RestoreHomeDirectory(previousHome);
      std::filesystem::remove_all(root);

      Assert::IsTrue(actual == expected);
    }

    TEST_METHOD(BinaryFileReturnsEmptyForMissingFiles)
    {
      std::wstring previousHome = Neuron::FileSys::GetHomeDirectory();
      auto const root = MakeUniqueTestRoot();
      std::filesystem::create_directories(root / L"GameData");
      Neuron::FileSys::SetHomeDirectory(root.wstring());

      Neuron::byte_buffer_t actual = Neuron::BinaryFile::ReadFile(L"missing.dat");

      RestoreHomeDirectory(previousHome);
      std::filesystem::remove_all(root);

      Assert::IsTrue(actual.empty());
    }

    TEST_METHOD(TextFileReadsWideTextFromAbsolutePath)
    {
      auto const root = MakeUniqueTestRoot();
      std::filesystem::create_directories(root);
      auto const filePath = root / L"message.txt";
      std::wstring expected = L"EarthRise NeuronCore";

      std::ofstream file(filePath, std::ios::binary);
      file.write(reinterpret_cast<const char*>(expected.data()), static_cast<std::streamsize>(expected.size() * sizeof(wchar_t)));
      file.close();

      std::wstring actual = Neuron::TextFile::ReadFile(filePath.wstring());

      std::filesystem::remove_all(root);

      Assert::IsTrue(actual == expected);
    }
  };
}
