#ifndef LTE_OS_h__
#define LTE_OS_h__

#include "String.h"
#include "Vector.h"

bool OS_ChangeDir(String const& dir);

void OS_ConfigureSignalHandlers();

bool OS_CreateDir(String const& path);

void OS_CreatePath(String const& path);

bool OS_FileExists(String const& path);

String OS_GetAppDir();

String OS_GetDocumentsDir();

String OS_GetUserDataPath();

String OS_GetWorkingDir();

bool OS_IsDir(String const& path);

bool OS_IsFile(String const& path);

Vector<String> OS_ListDir(String const& path);

void OS_MessageBox(String const& title, String const& message);

bool OS_Spawn(String const& path);

void OS_WriteDump(String const& path);

#endif
