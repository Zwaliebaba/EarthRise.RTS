#include "Profiler.h"
#include "Math.h"
#include "Module.h"
#include "ProgramLog.h"
#include "Renderer.h"
#include "String.h"
#include "Timer.h"
#include "Module/Settings.h"
#include <iostream>

const bool kDefaultFlush = false;

namespace {
#ifdef TELEMETRY
  HTELEMETRY g_context;

  struct ProfilingModule : public ModuleT {
    TmU8* arena;

    ProfilingModule() {
      tmLoadTelemetry(TM_LOAD_CHECKED_LIBRARY);
      tmStartup();         // Only call this once

      const unsigned int ARENA_SIZE = 2 * 1024 * 1024; // How much memory you want Telemetry to use
      arena = (TmU8*) malloc( ARENA_SIZE );
      tmInitializeContext( &g_context, (void*)arena, ARENA_SIZE );

      if (tmOpen(
            g_context,
            "Limit Theory",
            __DATE__ " " __TIME__,
            "localhost",
            TMCT_TCP,
            TELEMETRY_DEFAULT_PORT,
            TMOF_DEFAULT, 1000) != TM_OK )
        Log_Warning("Could not connect to telemetry server");
    }

    ~ProfilingModule() {
      tmClose(g_context);
      tmShutdownContext(g_context);
      tmShutdown();
      free(arena);
    }

    void Auto(float duration) {}

    void Flush() {}

    char const* GetName() const {
      return "Telemetry Profiler";
    }

    void SetFlushes(bool flushes) {}
    
    void Start() {}
    
    void Stop() {}

    void Update() {
      tmTick(g_context);
    }
  };

#else
  struct ProfilingModule : public ModuleT {
    bool active;
    bool flushes;

    size_t totalFrames;
    ::Timer timer;
    float maxTime;

    ProfilingModule() :
      active(false),
      flushes(kDefaultFlush),
      totalFrames(0),
      maxTime(-1)
      {}

    void Auto(float duration) {
      maxTime = duration;
      Start();
    }

    bool CanDelete() const {
      return false;
    }

    void Flush() {
      if (active)
        Renderer_Flush();
    }

    char const* GetName() const {
      return "Profiler";
    }

    void SetFlushes(bool flushes) {
      this->flushes = flushes;
    }

    void Start() {
      totalFrames = 1;
      timer.Reset();
      active = true;
    }

    void Stop() {
      active = false;
      float elapsed = timer.GetElapsed();
      float msPerFrame = totalFrames ? 1000.0f * elapsed / totalFrames : 0.0f;
      maxTime = -1;

      std::cout
        << "Profiler ran for " << elapsed << " seconds over "
        << totalFrames << " frames ("
        << ToString(msPerFrame) << " ms/frame)"
        << std::endl;
      std::cout << std::flush;
    }

    void Update() {
      if (active) {
        totalFrames++;
        if (maxTime > 0 && timer.GetElapsed() >= maxTime)
          Stop();
      }
    }
  };
#endif

  ProfilingModule* GetProfiler() {
    static std::shared_ptr<ProfilingModule> profiler;
    if (!profiler) {
      profiler = std::make_shared<ProfilingModule>();
      Module_RegisterGlobal(profiler);
    }
    return profiler.get();
  }
}

#ifdef ENABLE_PROFILING

void Profiler_Flush() {
  GetProfiler()->Flush();
}

void Profiler_Pop() {
}

void Profiler_Push(char const* name) {
  (void)name;
}

#endif

void Profiler_SetFlushes(bool flushes) {
  GetProfiler()->SetFlushes(flushes);
}

DefineFunction(Profiler_Auto) {
  GetProfiler()->Auto(args.duration);
}

DefineFunction(Profiler_Start) {
  GetProfiler()->Start();
}

DefineFunction(Profiler_Stop) {
  GetProfiler()->Stop();
}
