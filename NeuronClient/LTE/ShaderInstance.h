#ifndef LTE_ShaderInstance_h__
#define LTE_ShaderInstance_h__

#include "Color.h"
#include "DeclareFunction.h"
#include "Enum.h"
#include "Generic.h"
#include "Reference.h"
#include "Shader.h"
#include "V2.h"
#include "V3.h"
#include "V4.h"

#define RENDERSTATESWITCH_X                                                    \
  X(BlendModeAdditive)                                                         \
  X(BlendModeAlpha)                                                            \
  X(BlendModeComplementary)                                                    \
  X(BlendModeDisabled)                                                         \
  X(CullModeBackface)                                                          \
  X(CullModeDisabled)                                                          \
  X(WireframeOn)                                                               \
  X(WireframeOff)                                                              \
  X(ZBufferOn)                                                                 \
  X(ZBufferOff)                                                                \
  X(ZWritableOn)                                                               \
  X(ZWritableOff)

#define XTYPE RenderStateSwitch
#define XLIST RENDERSTATESWITCH_X
#include "LTE/XEnum.h"
#undef XLIST
#undef XTYPE

struct ShaderInstanceT : public RefCounted {
  struct ShaderInstanceData* d;

  ShaderInstanceT(Shader const& shader);
  ~ShaderInstanceT();

  ShaderInstance Clone() const;

  void Begin();
  void End();

  Shader const& GetShader() const;
  bool HasBlending() const;
  bool HasState(RenderStateSwitch) const;
  void Remove(char const* name);

  /* Immediate value setters. */
  ShaderInstanceT& SetCubeMap(
    char const* name,
    Generic<CubeMap, void> const& e);
  ShaderInstanceT& SetCubeMap(
    int index,
    Generic<CubeMap, void> const& e);

  ShaderInstanceT& SetFloat(char const* name, GenericFloat const& f);
  ShaderInstanceT& SetFloat(int index, GenericFloat const& f);

  ShaderInstanceT& SetFloat2(char const* name, GenericV2 const& v);
  ShaderInstanceT& SetFloat2(int index, GenericV2 const& v);

  ShaderInstanceT& SetFloat3(char const* name, GenericV3 const& v);
  ShaderInstanceT& SetFloat3(int index, GenericV3 const& v);

  ShaderInstanceT& SetFloat4(char const* name, GenericV4 const& v);
  ShaderInstanceT& SetFloat4(int index, GenericV4 const& v);

  ShaderInstanceT& SetInt(char const* name, GenericInt const& i);
  ShaderInstanceT& SetInt(int index, GenericInt const& i);

  ShaderInstanceT& SetMatrix(char const* name, Generic<Matrix const*, void> const& m);
  ShaderInstanceT& SetMatrix(int index, Generic<Matrix const*, void> const& m);

  ShaderInstanceT& SetState(RenderStateSwitch state);

  ShaderInstanceT& SetTexture2D(
    char const* name,
    Generic<Texture2D, void> const& t);
  ShaderInstanceT& SetTexture2D(
    int index,
    Generic<Texture2D, void> const& t);

  ShaderInstanceT& SetTexture3D(
    char const* name,
    Generic<Texture3D, void> const& t);
  ShaderInstanceT& SetTexture3D(
    int index,
    Generic<Texture3D, void> const& t);

  /* Convenient operator setters. */
  ShaderInstanceT& operator()(int index, Generic<CubeMap, void> const& e) {
    return SetCubeMap(index, e);
  }

  ShaderInstanceT& operator()(char const* name, Generic<CubeMap, void> const& e) {
    return SetCubeMap(name, e);
  }

  ShaderInstanceT& operator()(int index, GenericFloat const& f) {
    return SetFloat(index, f);
  }

  ShaderInstanceT& operator()(char const* name, GenericFloat const& f) {
    return SetFloat(name, f);
  }

  ShaderInstanceT& operator()(int index, GenericV2 const& v) {
    return SetFloat2(index, v);
  }

  ShaderInstanceT& operator()(char const* name, GenericV2 const& v) {
    return SetFloat2(name, v);
  }

  ShaderInstanceT& operator()(int index, GenericV3 const& v) {
    return SetFloat3(index, v);
  }

  ShaderInstanceT& operator()(char const* name, GenericV3 const& v) {
    return SetFloat3(name, v);
  }

  ShaderInstanceT& operator()(int index, GenericV4 const& v) {
    return SetFloat4(index, v);
  }

  ShaderInstanceT& operator()(char const* name, GenericV4 const& v) {
    return SetFloat4(name, v);
  }

  ShaderInstanceT& operator()(
    int index,
    Generic<Matrix const*, void> const& m)
  {
    return SetMatrix(index, m);
  }

  ShaderInstanceT& operator()(
    char const* name,
    Generic<Matrix const*, void> const& m)
  {
    return SetMatrix(name, m);
  }

  ShaderInstanceT& operator()(RenderStateSwitch state) {
    return SetState(state);
  }

  ShaderInstanceT& operator()(int index, Generic<Texture2D, void> const& t) {
    return SetTexture2D(index, t);
  }

  ShaderInstanceT& operator()(char const* name, Generic<Texture2D, void> const& t) {
    return SetTexture2D(name, t);
  }

  ShaderInstanceT& operator()(int index, Generic<Texture3D, void> const& t) {
    return SetTexture3D(index, t);
  }

  ShaderInstanceT& operator()(char const* name, Generic<Texture3D, void> const& t) {
    return SetTexture3D(name, t);
  }
};

DeclareFunction(ShaderInstance_Create, ShaderInstance,
  Shader, shader)

inline ShaderInstance ShaderInstance_Create(
  String const& vs,
  String const& ps)
{
  return ShaderInstance_Create(Shader_Create(vs, ps));
}

#endif
