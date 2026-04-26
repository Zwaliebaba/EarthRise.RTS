#include "ProgramLog.h"
#include "OS.h"
#include "StackFrame.h"
#include "Window.h"

#include <iostream>
#include <fstream>
#include <vector>

const uint kMaxEntries = 4096;
const bool kLogToFile = true;
String const kLogFile = OS_GetUserDataPath() + "logErrors.txt";

const LogLevel::Enum level = LogLevel::Everything;

namespace {
  std::vector<String>& GetEntryVector() {
    static std::vector<String> entries;
    return entries;
  }

  std::ofstream& GetLogFile() {
    static std::ofstream logFile(kLogFile.c_str());
    return logFile;
  }

  static void DoLog(String const& entry) {
    if (GetEntryVector().size() < kMaxEntries) {
      std::stringstream stream;
      stream << entry;
      String buffer;
      while (getline(stream, buffer)) {
        GetEntryVector().push_back(buffer);
        std::cout << entry << '\n';
        std::cout << std::flush;
        if (kLogToFile)
          GetLogFile() << entry << '\n' << std::flush;
      }
    }
  }
}

namespace LTE {
  void Log_Critical(String const& entry) {
#ifdef _NDEBUG
    GetWindow()->close();
#endif
    DoLog("[CRITICAL] (" + StackFrame_Get() + ") " + entry);
    DoLog("Shutting down due to critical Fatal.");
    OS_MessageBox("ERROR", "Shutting down due to critical Fatal:\n" + entry);
#ifdef _DEBUG
    DEBUG_ASSERT(false);
#endif
    exit(1);
  }

  void Log_Error(String const& entry) {
    if (level >= LogLevel::Errors)
      DoLog("[Error] (" + StackFrame_Get() + ") " + entry);
  }

  void Log_Event(String const& entry) {
    if (level >= LogLevel::Everything)
      DoLog("[Event] (" + StackFrame_Get() + ") " + entry);
  }

  void Log_Message(String const& entry) {
    if (level >= LogLevel::Everything)
      DoLog(entry);
  }

  void Log_Warning(String const& entry) {
    if (level >= LogLevel::Warnings)
      DoLog("[Warning] (" + StackFrame_Get() + ") " + entry);
  }

  size_t Log_GetEntries() {
    return GetEntryVector().size();
  }

  String const& Log_GetEntry(int index) {
    return GetEntryVector()[index];
  }
}
