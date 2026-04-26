#include "Interface.h"
#include "ClipRegion.h"
#include "Cursor.h"
#include "Widget.h"
#include "LTE/DrawState.h"
#include "LTE/Math.h"
#include "LTE/Module.h"
#include "LTE/Mouse.h"
#include "LTE/RenderPass.h"
#include "LTE/Renderer.h"
#include "LTE/Shader.h"
#include "LTE/StackFrame.h"
#include "LTE/Window.h"

namespace
{
  struct InterfaceImpl : InterfaceT
  {
    String name;
    Vector<Widget> widgets;

    InterfaceImpl(const String& name)
      : name(name) {}

    ~InterfaceImpl() override { Clear(); }

    void Add(const Widget& widget) override
    {
      widgets.push(widget);
      widget->Create();
    }

    void Clear() override
    {
      for (size_t i = 0; i < widgets.size(); ++i)
        widgets[i]->Clear();
      widgets.clear();
    }

    void Draw() override
    {
      SFRAME("InterfaceDraw");
      V2 windowSize = Window_Get()->GetSize();
      Cursor_Push(Mouse_GetPosImmediate(), Mouse_GetPosLast());
      ClipRegion_Push(0, windowSize);

      for (size_t i = 0; i < widgets.size(); ++i)
        widgets[i]->Draw();

      ClipRegion_Pop();
      Cursor_Pop();
    }

    void Update() override
    {
      SFRAME("InterfaceUpdate");
      V2 windowSize = Window_Get()->GetSize();
      Cursor_Push(Mouse_GetPos(), Mouse_GetPosLast());
      ClipRegion_Push(0, windowSize);

      WidgetUpdateState state;
      for (int i = 0; i < static_cast<int>(widgets.size()); ++i)
      {
        int index = widgets.size() - (i + 1);
        const Widget& widget = widgets[index];
        if (widget->deleted)
        {
          widget->Clear();
          widgets.eraseIndex(index);
          i--;
          continue;
        }

        widget->PrePosition();
        widget->pos = 0;
        widget->size = windowSize;
        widget->maxSize = widget->size;
        widget->PostPosition();
        widget->Update(state);
      }

      ClipRegion_Pop();
      Cursor_Pop();
    }
  };

  struct RenderPassInterface : RenderPassT
  {
    Interface interf;
    Shader compositor;
    DERIVED_TYPE_EX(RenderPassInterface)

    RenderPassInterface() {}

    RenderPassInterface(const Interface& _interf)
      : interf(_interf),
        compositor(Shader_Create("identity.jsl", "ui/composite.jsl")) {}

    const char* GetName() const override { return "Interface"; }

    void OnRender(DrawState* state) override
    {
      /* Draw interf to tertiary buffer. */
      {
        state->tertiary->Bind(0);
        Renderer_Clear(0);
        interf->Draw();
        state->tertiary->Unbind();
      }

      /* Final composite. */
      {
        DrawState_Link(compositor);
        RendererState s(BlendMode::Disabled, CullMode::Backface, false, false);
        state->secondary->Bind(0);
        (*compositor)("source", state->primary)("layer", state->tertiary);
        Renderer_SetShader(*compositor);
        Renderer_DrawFSQ();
        state->secondary->Unbind();
        state->Flip();
      }
    }
  };
}

DefineFunction(Interface_Create) { return new InterfaceImpl(args.name); }

DefineFunction(RenderPass_Interface) { return new RenderPassInterface(args.interf); }
