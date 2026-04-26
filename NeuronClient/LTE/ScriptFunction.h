#ifndef LTE_ScriptFunction_h__
#define LTE_ScriptFunction_h__

#include "AutoClass.h"
#include "Expression.h"
#include "String.h"
#include "Vector.h"

namespace LTE
{
  AutoClassDerived(ScriptFunctionT, RefCounted, String, name, Expression, expression, Type, returnType, Vector<Parameter>, parameters,
                   Function, function)

    ScriptFunctionT() {}

    void Call(void* returnValue, void** args);

    template <class T>
    void Call(T& returnValue)
    {
      DEBUG_ASSERT(returnType == Type_Get<T>());
      DEBUG_ASSERT(parameters.size() == 0);

      Call(&returnValue, static_cast<void**>(nullptr));
    }

    template <class T, class P0>
    void Call(T& returnValue, const P0& p0)
    {
      DEBUG_ASSERT(returnType == Type_Get<T>());
      DEBUG_ASSERT(parameters.size() == 1);
      DEBUG_ASSERT(parameters[0].type == Type_Get<P0>());

      void* args[] = { (void*)(&p0)};
      Call(&returnValue, args);
    }

    template <class T, class P0, class P1>
    void Call(T& returnValue, const P0& p0, const P1& p1)
    {
      DEBUG_ASSERT(returnType == Type_Get<T>());
      DEBUG_ASSERT(parameters.size() == 2);
      DEBUG_ASSERT(parameters[0].type == Type_Get<P0>());
      DEBUG_ASSERT(parameters[1].type == Type_Get<P1>());

      void* args[] = { (void*)(&p0), (void*)(&p1)};
      Call(&returnValue, args);
    }

    template <class T, class P0, class P1, class P2>
    void Call(T& returnValue, const P0& p0, const P1& p1, const P2& p2)
    {
      DEBUG_ASSERT(returnType == Type_Get<T>());
      DEBUG_ASSERT(parameters.size() == 3);
      DEBUG_ASSERT(parameters[0].type == Type_Get<P0>());
      DEBUG_ASSERT(parameters[1].type == Type_Get<P1>());
      DEBUG_ASSERT(parameters[2].type == Type_Get<P2>());

      void* args[] = { (void*)(&p0), (void*)(&p1), (void*)(&p2)};
      Call(&returnValue, args);
    }

    void VoidCall(void* returnValue)
    {
      DEBUG_ASSERT(parameters.size() == 0);

      bool constructRV = !returnValue && returnType->allocate;
      if (constructRV)
        returnValue = returnType->Allocate();
      Call(returnValue, static_cast<void**>(nullptr));
      if (constructRV)
        returnType->Deallocate(returnValue);
    }

    void VoidCall(void* returnValue, const DataRef& p0)
    {
      DEBUG_ASSERT(parameters.size() == 1);
      DEBUG_ASSERT(parameters[0].type == p0.type);

      void* args[] = {p0.data};
      bool constructRV = !returnValue && returnType->allocate;
      if (constructRV)
        returnValue = returnType->Allocate();
      Call(returnValue, args);
      if (constructRV)
        returnType->Deallocate(returnValue);
    }

    void VoidCall(void* returnValue, const DataRef& p0, const DataRef& p1)
    {
      DEBUG_ASSERT(parameters.size() == 2);
      DEBUG_ASSERT(parameters[0].type == p0.type);
      DEBUG_ASSERT(parameters[1].type == p1.type);

      void* args[] = {p0.data, p1.data};
      bool constructRV = !returnValue && returnType->allocate;
      if (constructRV)
        returnValue = returnType->Allocate();
      Call(returnValue, args);
      if (constructRV)
        returnType->Deallocate(returnValue);
    }

    void VoidCall(void* returnValue, const DataRef& p0, const DataRef& p1, const DataRef& p2)
    {
      DEBUG_ASSERT(parameters.size() == 3);
      DEBUG_ASSERT(parameters[0].type == p0.type);
      DEBUG_ASSERT(parameters[1].type == p1.type);
      DEBUG_ASSERT(parameters[2].type == p2.type);

      void* args[] = {p0.data, p1.data, p2.data};
      bool constructRV = !returnValue && returnType->allocate;
      if (constructRV)
        returnValue = returnType->Allocate();
      Call(returnValue, args);
      if (constructRV)
        returnType->Deallocate(returnValue);
    }
  };
}

#endif
