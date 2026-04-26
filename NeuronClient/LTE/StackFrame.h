#ifndef LTE_StackFrame_h__
#define LTE_StackFrame_h__

#define _FNAME_ __FUNCTION__

String StackFrame_Get();

void StackFrame_Print();

void StackFrame_Pop();

void StackFrame_Push(const char* func, const char* file, int line, const char* annotation);

struct ScopedFrame
{
  int i;

  ScopedFrame(const char* func, const char* file, int line, const char* annotation = "")
    : i(0) { StackFrame_Push(func, file, line, annotation); }

  ~ScopedFrame() { StackFrame_Pop(); }
};

#ifdef _DEBUG

#define AUTO_FRAME ScopedFrame __sframe(_FNAME_, __FILE__, __LINE__, _FNAME_)
#define SFRAME(x) ScopedFrame __sframe(_FNAME_, __FILE__, __LINE__, x)
#define FRAME(x)                                                       \
  for (ScopedFrame __f(_FNAME_, __FILE__, __LINE__, x); __f.i < 1; __f.i++)

#else

#define AUTO_FRAME __noop()
#define SFRAME(x) __noop(x)
#define FRAME(x) __noop(x)

#endif

#endif
