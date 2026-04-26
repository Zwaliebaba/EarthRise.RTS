#include "../RenderPasses.h"

#include "Game/Object.h"
#include "Game/Graphics/RenderStyles.h"

#include "LTE/DrawState.h"
#include "LTE/RenderStyle.h"
#include "LTE/StackFrame.h"
#include "LTE/Texture2D.h"

namespace
{
  struct BlendedPass : RenderPassT
  {
    RenderStyle style;
    DERIVED_TYPE_EX(BlendedPass)

    BlendedPass()
      : style(RenderStyle_Default(true)) {}

    const char* GetName() const override { return "Blended"; }

    void OnRender(DrawState* state) override
    {
      RenderStyle_Push(style);
      state->primary->Bind(0);
      for (size_t i = 0; i < state->visible.size(); ++i)
        static_cast<ObjectT*>(state->visible[i])->OnDraw(state);
      state->primary->Unbind();
      RenderStyle_Pop();
    }
  };
}

DefineFunction(RenderPass_Blended) { return new BlendedPass; }
