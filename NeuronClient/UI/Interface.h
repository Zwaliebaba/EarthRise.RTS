#ifndef UI_Interface_h__
#define UI_Interface_h__

#include "Common.h"
#include "LTE/DeclareFunction.h"
#include "LTE/RenderPass.h"

struct InterfaceT : RefCounted
{
  ~InterfaceT() override {}

  virtual void Add(const Widget& widget) = 0;
  virtual void Clear() = 0;
  virtual void Draw() = 0;
  virtual void Update() = 0;
};

DeclareFunction(Interface_Create, Interface, String, name)

DeclareFunction(RenderPass_Interface, RenderPass, Interface, interf)

#endif
