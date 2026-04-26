#ifndef LTE_Expression_h__
#define LTE_Expression_h__

#include "Array.h"
#include "AutoPtr.h"
#include "BaseType.h"
#include "Data.h"
#include "Function.h"
#include "Pointer.h"
#include "Reference.h"

namespace LTE
{
  struct CompileEnvironment;
  struct Environment;

  struct ExpressionT : RefCounted
  {
    BASE_TYPE(ExpressionT)

    virtual String Emit(Vector<String>& context) const { return ""; }

    virtual void Evaluate(void* returnValue, Environment& env) const = 0;

    virtual void* GetLValue(Environment& env) const { return nullptr; }

    virtual Type GetType() const = 0;

    virtual bool IsConstant(CompileEnvironment& env) const = 0;

    virtual bool IsLValue() const { return false; }

    void Evaluate(void* returnValue) const;

    template <class T>
    void GetResult(T& t) const
    {
      if (GetType() != Type_Get<T>())
        Fatal("Expression return type differs from expected type");
      Evaluate(&t);
    }

    FIELDS {}
  };

  Expression Expression_Compile(const StringList& list);

  Expression Expression_Compile(const StringList& list, CompileEnvironment& env, Vector<String>* locals = nullptr);
}

#endif
