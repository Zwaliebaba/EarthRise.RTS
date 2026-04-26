#include "../RenderPasses.h"

#include "LTE/DrawState.h"
#include "LTE/Renderer.h"
#include "LTE/Texture2D.h"

namespace
{
  struct Clear : RenderPassT
  {
    V4 value;
    DERIVED_TYPE_EX(Clear)

    Clear() {}

    Clear(const V4& value)
      : value(value) {}

    const char* GetName() const override { return "Clear"; }

    void OnRender(DrawState* state) override
    {
      Renderer_ResetCounters();
      state->primary->Bind(0);
      Renderer_Clear(value);
      state->primary->Unbind();

      for (uint i = 0; i < 2; ++i)
      {
        state->smallColor[i]->Bind(0);
        Renderer_Clear();
        state->smallColor[i]->Unbind();
      }
    }
  };

  struct ClearDepth : RenderPassT
  {
    DERIVED_TYPE_EX(ClearDepth)

    const char* GetName() const override { return "Clear Depth"; }

    void OnRender(DrawState* state) override
    {
      state->primary->Bind(0);
      Renderer_ClearDepth();
      state->primary->Unbind();
    }
  };
}

DefineFunction(RenderPass_Clear) { return new Clear(args.value); }

DefineFunction(RenderPass_ClearDepth) { return new ClearDepth; }
