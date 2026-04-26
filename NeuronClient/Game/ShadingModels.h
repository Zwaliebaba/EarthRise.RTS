#ifndef ShadingModels_h__
#define ShadingModels_h__

#include "LTE/Generic.h"

ShaderInstance ShadingModel_Debug();

ShaderInstance ShadingModel_FlatColor(Color const& color);

ShaderInstance ShadingModel_Fresnel(
  Generic<Texture2D> const& albedoMap,
  Generic<Texture2D> const& normalMap);

ShaderInstance ShadingModel_Lambert(
  Generic<Texture2D> const& albedoMap,
  Generic<Texture2D> const& normalMap);

ShaderInstance ShadingModel_Metal(
  Generic<Texture2D> const& albedo,
  Generic<Texture2D> const& normal);

ShaderInstance ShadingModel_Skybox();

ShaderInstance ShadingModel_Water();

#endif
