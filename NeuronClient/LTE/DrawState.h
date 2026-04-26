#ifndef LTE_DrawState_h__
#define LTE_DrawState_h__

#include "CubeMap.h"
#include "DeclareFunction.h"
#include "Pushable.h"
#include "Stack.h"
#include "Texture2D.h"
#include "Vector.h"

#undef DrawState

struct DrawState : Pushable<DrawState>
{
  Stack<View*> view;
  Stack<CubeMap> envMap;
  Stack<CubeMap> envMapLF;

  /* Buffer pointers. */
  Texture2D primary;
  Texture2D secondary;
  Texture2D tertiary;

  /* Color. */
  Texture2D color[3];
  Texture2D smallColor[2];

  /* Depth. */
  Texture2D depth;

  /* Visibility. */
  Vector<void*> lights;
  Vector<void*> visible;

  void Flip() { Swap(primary, secondary); }
};

LT_API const Data& DrawState_Get(const String& name);
LT_API void DrawState_Inject(const Shader& shader);

LT_API void DrawState_Link(const Shader& shader);
LT_API void DrawState_Link(const ShaderInstance& shaderState);

DeclareFunctionNoParams(DrawState_Clear, void)

DeclareFunction(DrawState_Pop, void, String, name)

DeclareFunction(DrawState_Push, void, String, name, Data, data)

#endif
