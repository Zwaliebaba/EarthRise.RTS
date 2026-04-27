#include "Loader.h"
#include "Module.h"
#include "Pointer.h"
#include "Vector.h"

namespace {
  struct LoaderHandler : public ModuleT {
    Vector<void (*)()> toLoad;
    Vector<void (*)()> toUnload;

    ~LoaderHandler() {
      for (size_t i = 0; i < toUnload.size(); ++i)
        toUnload[i]();
    }

    char const* GetName() const {
      return "Loader Handler";
    }

    void Update() {
      for (size_t i = 0; i < toLoad.size(); ++i)
        toLoad[i]();
      toLoad.clear();
    }
  };

  LoaderHandler* GetHandler() {
    static std::shared_ptr<LoaderHandler> handler;
    if (!handler) {
      handler = std::make_shared<LoaderHandler>();
      Module_RegisterGlobal(handler);
    }
    return handler.get();
  }
}

namespace LTE {
  bool RegisterLoader(void (*loader)()) {
    GetHandler()->toLoad.push(loader);
    return true;
  }

  bool RegisterUnloader(void (*unloader)()) {
    GetHandler()->toUnload.push(unloader);
    return true;
  }
}
