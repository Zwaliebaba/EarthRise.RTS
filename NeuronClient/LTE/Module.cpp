#include "Module.h"

namespace {
  std::vector<Module>& GetModules() {
    static std::vector<Module> gModules;
    return gModules;
  }
}

void Module_RegisterGlobal(Module const& module) {
  DEBUG_ASSERT(std::find(GetModules().begin(), GetModules().end(), module) == GetModules().end());
  GetModules().push_back(module);
}

void Module_UpdateGlobal() {
  for (size_t i = 0; i < GetModules().size(); ++i)
      GetModules()[i]->Update();
}
