#ifndef LTE_ProgramLog_h__
#define LTE_ProgramLog_h__

#include "String.h"

namespace LTE {
  namespace LogLevel {
    enum Enum {
      Nothing,
      Errors,
      Warnings,
      Everything
    };
  }

  void Log_Critical(String const& entry);
  void Log_Error(String const& entry);
  void Log_Event(String const& entry);
  void Log_Message(String const& entry);
  void Log_Warning(String const& entry);

  size_t Log_GetEntries();
  String const& Log_GetEntry(int index);
}

#endif
