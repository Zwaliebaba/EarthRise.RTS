#include "LTE/AutoClass.h"
#include "LTE/Data.h"
#include "LTE/Function.h"
#include "LTE/Job.h"
#include "LTE/ProgramLog.h"
#include "LTE/Script.h"
#include "LTE/Thread.h"

TypeAlias(Reference<ThreadT>, Thread);

AutoClassDerived(ScriptedJob, JobT, Data, object, ScriptFunction, function, void*, returnValue)

  ScriptedJob() {}

  ~ScriptedJob() override
  {
    if (returnValue)
      function->returnType->Deallocate(returnValue);
  }

  const char* GetName() const override { return &function->name.front(); }

  void OnBegin() override
  {
    if (function->returnType->allocate)
      returnValue = function->returnType->Allocate();
  }

  void OnRun(uint units) override { function->VoidCall(returnValue, object); }
};

FreeFunction(Thread, Thread_Create, "Create a thread that executes the function named 'function' in 'object'", Data, object, String,
             function)
{
  auto type = object.type->GetAux().Convert<ScriptType>();
  ScriptFunction fn = type->GetFunction(function);
  if (!fn)
  {
    Log_Error(Stringize() | "Thread object '" | object.type->name | "' has no function '" | function | "'");
    return nullptr;
  }

  return Thread_Create(new ScriptedJob(object, fn, nullptr));
}

FreeFunction(bool, Thread_IsFinished, "Return whether 'thread' has finished executing", Thread, thread) { return thread->IsFinished(); }
FunctionAlias(Thread_IsFinished, IsFinished);

FreeFunction(Data, Thread_GetResult, "Get the return value (if any) of 'thread'", Thread, thread)
{
  auto job = static_cast<ScriptedJob*>(thread->GetJob().get());
  return Data(job->function->returnType, job->returnValue);
}

FunctionAlias(Thread_GetResult, GetResult);
