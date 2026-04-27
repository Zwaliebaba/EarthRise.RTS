#include "RenderPass.h"

#include "Profiler.h"
#include "Renderer.h"

#include "Module/Settings.h"

void RenderPassT::Render(DrawState* state) {
  if (Settings_Bool((String)"Graphics/" + ToString(), true)()) {
    {
      OnRender(state);
      Profiler_Flush();
    }
  }
}
