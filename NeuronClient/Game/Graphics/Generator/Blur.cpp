#include "../Generators.h"

#include "LTE/CubeMap.h"
#include "LTE/ShaderInstance.h"

namespace {
  CubeMap Generate(Generator_Blur_Args const& args) {
    static Shader shader = Shader_Create("identity.jsl", "cubemap/blur.jsl");

    CubeMap const& source = args.source();
    CubeMap self = CubeMap_Create(args.resolution, source->GetFormat());
    (*shader)
      ("radius", args.radius)
      ("source", args.source())
      ("samples", (int)args.samples);
    self->GenerateFromShader(shader);
    return self;
  }
}

DefineFunction(Generator_Blur) {
  return Cached(Bind(FreeFn(Generate), Generator_Blur_Args(args)));
}
