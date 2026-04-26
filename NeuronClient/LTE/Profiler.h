#ifndef LTE_Profiler_h__
#define LTE_Profiler_h__

#include "Common.h"
#include "DeclareFunction.h"

#define ENABLE_PROFILING

#ifdef ENABLE_PROFILING

void Profiler_Flush();
void Profiler_Pop();
void Profiler_Push(char const* name);

#else

inline void Profiler_Flush() {}
inline void Profiler_Pop() {}
inline void Profiler_Push(char const* name) {}

#endif

DeclareFunction(Profiler_Auto, void,
  float, duration)
DeclareFunctionNoParams(Profiler_Start, void)
DeclareFunctionNoParams(Profiler_Stop, void)

void Profiler_SetFlushes(bool flushes);

#endif
