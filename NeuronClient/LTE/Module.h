#pragma once

#include <memory>

typedef std::shared_ptr<struct ModuleT> Module;

struct ModuleT
{
  virtual ~ModuleT() {}

  virtual const char* GetName() const = 0;
  virtual void Update() = 0;
};

void Module_RegisterGlobal(const Module& module);
void Module_UpdateGlobal();

