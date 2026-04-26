#ifndef LTE_Module_h__
#define LTE_Module_h__

#include "Reference.h"

struct ModuleT : RefCounted
{
  ~ModuleT() override {}

  virtual const char* GetName() const = 0;
  virtual void Update() = 0;
};

void Module_RegisterGlobal(const Module& module);
void Module_UpdateGlobal();

#endif
