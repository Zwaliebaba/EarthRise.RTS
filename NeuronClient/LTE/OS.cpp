#include "OS.h"
#include "Profiler.h"
#include "ProgramLog.h"
#include "String.h"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <sys/stat.h>

#include "process.h"
#include "Shlobj.h"
#include "DbgHelp.h"
#include "windows.h"
#include "Tchar.h"
#include "Direct.h"

#include <filesystem>

#undef CreateDirectory
#undef MessageBox

void IntHandler(int value)
{
  exit(0);
}

void SegHandler(int value)
{
  Fatal("Access Violation");
  exit(0);
}

LONG _stdcall Win32SegHandler(EXCEPTION_POINTERS* pExcept)
{
  SegHandler(0);
  return 0;
}

bool OS_ChangeDir(const String& path)
{
  return SetCurrentDirectoryA(path) != 0;
}

void OS_ConfigureSignalHandlers()
{
}

bool OS_CreateDir(const String& path)
{
  return CreateDirectoryA(path, nullptr) != 0;
}

void OS_CreatePath(const String& path)
{
  if (!path.size())
    return;

  std::stringstream stream(path);
  String buf;
  String partialPath;
  std::vector<String> partials;
  while (getline(stream, buf, '/')) { partials.push_back(buf); }
  for (size_t i = 0; i + 1 < partials.size(); ++i)
  {
    partialPath += partials[i] + '/';
    OS_CreateDir(partialPath);
  }

  if (path.back() == '/')
    OS_CreateDir(path);
}

bool OS_FileExists(const String& path)
{
  struct stat s;
  return stat(path, &s) == 0;
}

String OS_GetAppDir()
{
  char* path = nullptr;
  size_t len = 0;
  _dupenv_s(&path, &len, "APPDATA");
  std::string result = path ? std::string(path) : std::string{};
  free(path);
  return result;
}

String OS_GetDocumentsDir()
{
  char* profile = nullptr;
  size_t len = 0;
  _dupenv_s(&profile, &len, "USERPROFILE");
  String result = profile ? String(profile) + "\\Documents" : String(".\\data");
  free(profile);
  return result;
}

String OS_GetUserDataPath()
{
  static bool created = false;
  if (!created)
  {
    created = true;
    OS_CreateDir(".\\cache\\");
  }
  return ".\\cache\\";
}

String OS_GetWorkingDir()
{
  char path[1024];
  if (!_getcwd(path, sizeof(path)))
    return "";
  return path;
}

bool OS_IsDir(const String& path)
{
  struct stat s;
  return stat(path, &s) == 0 && (s.st_mode & S_IFDIR);
}

bool OS_IsFile(const String& path)
{
  struct stat s;
  return stat(path, &s) == 0 && (s.st_mode & S_IFREG);
}

Vector<String> OS_ListDir(const String& path)
{
  Vector<String> result;
  for (std::filesystem::directory_iterator it((char const*)path), end; it != end; ++it)
    result.push(it->path().filename().string().c_str());
  return result;
}

void OS_MessageBox(const String& title, const String& message)
{
  MessageBoxA(nullptr, message, title, MB_OK);
}

bool OS_Spawn(const String& path)
{
  char* const argv[] = {_strdup(path), nullptr};
  return _spawnl(_P_NOWAIT, path, path, NULL) != -1;
}

void OS_WriteDump(const String& path)
{
  using pDumpFn = BOOL(_stdcall *)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                   PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                   PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

  HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  HMODULE h = LoadLibrary(_T("DbgHelp.dll"));
  DEBUG_ASSERT(h);
  if (!h)
    return;

  auto dumpFn = (pDumpFn)GetProcAddress(h, "MiniDumpWriteDump");
  if (!dumpFn)
    return;

  if (hFile && hFile != INVALID_HANDLE_VALUE)
  {
    BOOL rv = (*dumpFn)(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, nullptr, nullptr, nullptr);
    CloseHandle(hFile);
  }
  FreeLibrary(h);
}
