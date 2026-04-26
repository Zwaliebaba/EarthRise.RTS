#include "Shader.h"
#include "AutoPtr.h"
#include "CubeMap.h"
#include "GL.h"
#include "Location.h"
#include "Map.h"
#include "Matrix.h"
#include "Pointer.h"
#include "Program.h"
#include "ProgramLog.h"
#include "Renderer.h"
#include "StackFrame.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "V4.h"
#include <sstream>
#include <iostream>

const String kVersionDirective = "#version 120\n";
constexpr uint kTextureUnits = 16;

namespace
{
  using ShaderObject = Reference<struct ShaderObjectT>;
  using ProgramObject = Reference<struct ProgramObjectT>;
  using ShaderMap = Map<String, ShaderObject>;
  using ProgramMap = Map<String, ProgramObject>;
  using LocationCache = Map<const char*, int>;

  GL_Program gActiveProgram = GL_NullProgram;
  Pointer<ShaderT> gActiveShader;

  ShaderMap& GetShaderCache()
  {
    static ShaderMap map;
    return map;
  }

  ProgramMap& GetProgramCache()
  {
    static ProgramMap map;
    return map;
  }

  bool ShouldWarnMissingUniform(const char* name)
  {
    const char* optionalUniforms[] = {
      "center", "color1", "color2", "decal", "halfTexel", "offset", "orientation1", "prepass", "roughness", "seed", "texelScale", "texture"
    };

    for (size_t i = 0; i < std::size(optionalUniforms); ++i)
    {
      if (!std::strcmp(name, optionalUniforms[i]))
        return false;
    }

    return true;
  }

  /* Run a manual preprocessor on the shader code to support #include. */
  String JSLPreprocess(const String& code)
  {
    std::stringstream parsed;
    std::stringstream codestream(code);
    String buf;

    while (getline(codestream, buf))
    {
      if (buf.size() && buf.front() == '#')
      {
        Vector<String> tokens;
        String_Split(tokens, buf.substr(1), ' ');

        if (!tokens.size())
        {
          parsed << buf << '\n';
          continue;
        }

        if (tokens[0] == "include")
        {
          DEBUG_ASSERT(tokens.size() == 2);
          parsed << JSLPreprocess(Location_Shader("common\\" + tokens[1])->ReadAscii());
        }

        else if (tokens[0] == "output")
        {
          DEBUG_ASSERT(tokens.size() == 4);
          int index = FromString<int>(tokens[1]);
          const String& varType = tokens[2];
          int components = 1;
          if (varType.contains('2'))
            components = 2;
          else if (varType.contains('3'))
            components = 3;
          else if (varType.contains('4'))
            components = 4;

          parsed << "#define " << tokens[3] << " gl_FragData[" << index << "].";
          parsed << String("xyzw").substr(0, components);
          parsed << '\n';
        }

        else
          parsed << buf << '\n';
      }
      else
        parsed << buf << '\n';
    }
    return parsed.str();
  }

  struct ShaderObjectT : RefCounted
  {
    GL_Shader id;
    String path;
    String source;
    int version;

    ShaderObjectT()
      : version(0) {}

    ~ShaderObjectT() override
    {
      if (!Program_InStaticSection())
      {
        GL_DeleteShader(id);
        if (path.size() && GetShaderCache().get(path))
          GetShaderCache().erase(path);
      }
    }

    bool Compile()
    {
      SFRAME("Compile GPU Shader");
      version++;
      GL_ShaderSource(id, source);
      GL_CompileShader(id);

      /* Check the compilation status to see if there was an Fatal. */
      int compileStatus = GL_GetShaderI(id, GL_ShaderProperty::CompileStatus);

      if (compileStatus == 0)
      {
        Log_Error("Failed to compile shader.");
        PrintSource();
        Log_Message("Compiler Log:");
        Log_Message(GetLog());
        getchar();
        return false;
      }
      return true;
    }

    String GetLog() const
    {
      String log;
      GL_GetShaderInfoLog(id, log);
      return log;
    }

    void PrintSource()
    {
      Vector<String> splitSource;
      String_Split(splitSource, source, '\n');
      for (size_t i = 0; i < splitSource.size(); ++i)
        Log_Message(ToString(i) + '\t' + splitSource[i]);
    }
  };

  ShaderObject ShaderObject_Create(const String& code, GL_ShaderType::Enum type)
  {
    ShaderObject self = new ShaderObjectT;
    self->id = GL_CreateShader(type);
    self->source = kVersionDirective + JSLPreprocess(code);
    if (!self->Compile())
      return nullptr;
    return self;
  }

  ShaderObject ShaderObject_Load(const String& path, GL_ShaderType::Enum type)
  {
    if (GetShaderCache().get(path))
      return GetShaderCache()[path];

    String code = Location_Shader(path)->ReadAscii();
    if (code.empty())
    {
      Log_Error("Failed to load shader <" + path + ">");
      return nullptr;
    }

    ShaderObject self = ShaderObject_Create(code, type);
    if (self)
    {
      GetShaderCache()[path] = self;
      self->path = path;
    }
    else
      Log_Error("Failed to load shader <" + path + ">");

    return self;
  }

  struct ProgramObjectT : RefCounted
  {
    ShaderObject vertShader;
    ShaderObject fragShader;
    GL_Program id;
    String path;
    int textureUnitIndex;
    int version;

    LocationCache uniforms;
    int mWorld;
    int mView;
    int mProj;
    int mWorldIT;
    int mWVP;

    ProgramObjectT()
      : textureUnitIndex(0),
        version(0),
        mWorld(-1),
        mView(-1),
        mProj(-1),
        mWorldIT(-1),
        mWVP(-1) {}

    ~ProgramObjectT() override
    {
      if (!Program_InStaticSection())
      {
        GL_DeleteProgram(id);
        if (gActiveProgram == id)
          Shader_UseFixedFunction();
        if (path.size() && GetProgramCache().get(path))
          GetProgramCache().erase(path);
      }
    }

    void BindGlobalAttributes()
    {
      BindInput(0, "vertex_position");
      BindInput(1, "vertex_normal");
      BindInput(2, "vertex_uv");
      BindInput(3, "vertex_color");
      BindOutput(0, "fragment_color0");
      BindOutput(1, "fragment_linearDepth");
    }

    void BindInput(size_t attribIndex, const char* name)
    {
      DEBUG_ASSERT(attribIndex < kAttribArrays);
      GL_BindAttribLocation(id, attribIndex, name);
    }

    void BindOutput(size_t bufferIndex, const char* name)
    {
      DEBUG_ASSERT(bufferIndex < GL_MAX_DRAW_BUFFERS);
      GL_BindFragDataLocation(id, bufferIndex, name);
    }

    void CacheWVP()
    {
      /* Cache the locations of WVP matrices. */
      mWorld = GL_GetUniformLocation(id, "WORLD");
      mView = GL_GetUniformLocation(id, "VIEW");
      mProj = GL_GetUniformLocation(id, "PROJ");
      mWorldIT = GL_GetUniformLocation(id, "WORLDIT");
      mWVP = GL_GetUniformLocation(id, "WVP");
    }

    String GetLog() const
    {
      String log;
      GL_GetProgramInfoLog(id, log);
      return log;
    }

    int GetUniformLocation(const char* name, bool warn)
    {
      int* it = uniforms.get(name);
      if (it)
        return *it;

      int index = GL_GetUniformLocation(id, name);
      uniforms[name] = index;

      if (warn && index < 0 && ShouldWarnMissingUniform(name))
      {
        String warning = Stringize() | "Unused variable " | name | " in Shader(" | vertShader->path | ", " | fragShader->path | ")";
        Log_Warning(warning);
      }

      return index;
    }

    void Link()
    {
      SFRAME("Link GPU Program");
      GL_LinkProgram(id);

      if (GL_GetProgramI(id, GL_ProgramProperty::LinkStatus) == 0)
      {
        Log_Error("Failed to link program.");
        Log_Error(GetLog());
        vertShader->PrintSource();
        fragShader->PrintSource();
      }

      CacheWVP();
      version = vertShader->version ^ fragShader->version;
    }
  };

  ProgramObject ProgramObject_Create(const ShaderObject& vertex, const ShaderObject& fragment)
  {
    ProgramObject self = new ProgramObjectT;
    self->id = GL_CreateProgram();
    if (self->id == GL_NullProgram)
      return nullptr;

    self->vertShader = vertex;
    self->fragShader = fragment;
    GL_AttachShader(self->id, vertex->id);
    GL_AttachShader(self->id, fragment->id);
    self->BindGlobalAttributes();
    self->Link();
    return self;
  }

  ProgramObject ProgramObject_Load(const String& vertPath, const String& fragPath)
  {
    String programPath = vertPath + "?" + fragPath;
    if (GetProgramCache().get(programPath))
      return GetProgramCache()[programPath];

    ShaderObject vs = ShaderObject_Load("vertex/" + vertPath, GL_ShaderType::Vertex);
    if (!vs)
      return nullptr;

    ShaderObject fs = ShaderObject_Load("fragment/" + fragPath, GL_ShaderType::Fragment);
    if (!fs)
      return nullptr;

    ProgramObject self = ProgramObject_Create(vs, fs);
    if (self)
    {
      GetProgramCache()[programPath] = self;
      self->path = programPath;
    }
    else
      Log_Error("Failed to load program <" + vertPath + ", " + fragPath + ">");

    return self;
  }

  struct ShaderImpl : ShaderT
  {
    ProgramObject program;

    ~ShaderImpl() override
    {
      if (gActiveShader == this)
        gActiveShader = nullptr;
    }

    void BindInput(size_t attribIndex, const char* name) override { program->BindInput(attribIndex, name); }

    void BindOutput(size_t bufferIndex, const char* name) override { program->BindOutput(bufferIndex, name); }

    void BindMatrices(const Matrix& world, const Matrix& view, const Matrix& proj, const Matrix& worldIT, const Matrix& WVP) override
    {
      if (program->mWorld >= 0)
        SetMatrix(program->mWorld, &world);
      if (program->mView >= 0)
        SetMatrix(program->mView, &view);
      if (program->mProj >= 0)
        SetMatrix(program->mProj, &proj);
      if (program->mWorldIT >= 0)
        SetMatrix(program->mWorldIT, &worldIT);
      if (program->mWVP >= 0)
        SetMatrix(program->mWVP, &WVP);
    }

    bool Create(const String& vertCode, const String& fragCode) override
    {
      ShaderObject vertex = ShaderObject_Create(vertCode, GL_ShaderType::Vertex);
      if (!vertex)
        return false;

      ShaderObject fragment = ShaderObject_Create(fragCode, GL_ShaderType::Fragment);
      if (!fragment)
        return false;

      program = ProgramObject_Create(vertex, fragment);
      return program != nullptr;
    }

    int GetTextureUnit()
    {
      int index = program->textureUnitIndex++;
      program->textureUnitIndex = program->textureUnitIndex % kTextureUnits;
      return index;
    }

    int GetUniformLocation(const char* name) override { return program->GetUniformLocation(name, true); }

    int QueryUniformLocation(const char* name) override { return program->GetUniformLocation(name, false); }

    void PrintLogs() const override
    {
      std::cout << ">>> Vertex Shader:\n" << program->vertShader->GetLog() << "\n\n" << ">>> Fragment Shader:\n" << program->fragShader->
        GetLog() << "\n\n" << ">>> Program:\n" << program->GetLog() << "\n\n";
    }

    void Relink() override { program->Link(); }

    ShaderT& SetCubeMap(const char* name, const CubeMap& cubeMap) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetCubeMap(varIndex, cubeMap);
    }

    ShaderT& SetCubeMap(int varIndex, const CubeMap& cubeMap) override
    {
      Use();
      int unit = GetTextureUnit();
      GL_Uniform(varIndex, unit);
      GL_ActiveTexture(unit);
      cubeMap->Bind();
      GL_ActiveTexture(0);
      return *this;
    }

    ShaderT& SetFloat(const char* name, float f) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloat(varIndex, f);
    }

    ShaderT& SetFloat(int varIndex, float f) override
    {
      Use();
      GL_Uniform(varIndex, f);
      return *this;
    }

    ShaderT& SetFloatArray(const char* name, const float* data, size_t size) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloatArray(varIndex, data, size);
    }

    ShaderT& SetFloatArray(int varIndex, const float* data, size_t size) override
    {
      Use();
      GL_UniformArray1(varIndex, size, data);
      return *this;
    }

    ShaderT& SetFloat2(const char* name, const V2& v) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloat2(varIndex, v);
    }

    ShaderT& SetFloat2(int varIndex, const V2& v) override
    {
      Use();
      GL_Uniform(varIndex, v.x, v.y);
      return *this;
    }

    ShaderT& SetFloat3(const char* name, const V3& v) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloat3(varIndex, v);
    }

    ShaderT& SetFloat3(int varIndex, const V3& v) override
    {
      Use();
      GL_Uniform(varIndex, v.x, v.y, v.z);
      return *this;
    }

    ShaderT& SetFloat4(const char* name, const V4& v) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloat4(varIndex, v);
    }

    ShaderT& SetFloat4(int varIndex, const V4& v) override
    {
      Use();
      GL_Uniform(varIndex, v.x, v.y, v.z, v.w);
      return *this;
    }

    ShaderT& SetFloat3Array(const char* name, const V3* data, size_t size) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloat3Array(varIndex, data, size);
    }

    ShaderT& SetFloat3Array(int varIndex, const V3* data, size_t size) override
    {
      Use();
      GL_UniformArray3(varIndex, size, (const float*)data);
      return *this;
    }

    ShaderT& SetFloat4Array(const char* name, const V4* data, size_t size) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetFloat4Array(varIndex, data, size);
    }

    ShaderT& SetFloat4Array(int varIndex, const V4* data, size_t size) override
    {
      Use();
      GL_UniformArray4(varIndex, size, (const float*)data);
      return *this;
    }

    ShaderT& SetMatrix(const char* name, const Matrix* m) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetMatrix(varIndex, m);
    }

    ShaderT& SetMatrix(int varIndex, const Matrix* m) override
    {
      Use();
      GL_UniformMatrix4(varIndex, &(m->e[0]));
      return *this;
    }

    ShaderT& SetInt(const char* name, int i) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetInt(varIndex, i);
    }

    ShaderT& SetInt(int varIndex, int i) override
    {
      Use();
      GL_Uniform(varIndex, i);
      return *this;
    }

    ShaderT& SetTexture2D(const char* name, const Texture2D& t) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetTexture2D(varIndex, t);
    }

    ShaderT& SetTexture2D(int varIndex, const Texture2D& t) override
    {
      Use();
      int unit = GetTextureUnit();
      GL_Uniform(varIndex, unit);
      if (t)
        t->BindInput(unit);
      else
      {
        GL_ActiveTexture(unit);
        GL_BindTexture(GL_TextureTargetBindable::T2D, GL_NullTexture);
      }
      GL_ActiveTexture(0);
      return *this;
    }

    ShaderT& SetTexture3D(const char* name, const Texture3D& t) override
    {
      int varIndex = GetUniformLocation(name);
      if (varIndex < 0)
        return *this;
      return SetTexture3D(varIndex, t);
    }

    ShaderT& SetTexture3D(int varIndex, const Texture3D& t) override
    {
      Use();
      int unit = GetTextureUnit();
      GL_Uniform(varIndex, unit);
      if (t)
        t->Bind(unit);
      else
      {
        GL_ActiveTexture(unit);
        GL_BindTexture(GL_TextureTargetBindable::T3D, GL_NullTexture);
      }
      GL_ActiveTexture(0);
      return *this;
    }

    void Use() override
    {
      if (gActiveProgram != program->id)
      {
        gActiveProgram = program->id;
        gActiveShader = this;
        GL_UseProgram(program->id);
      }
    }
  };
}

DefineFunction(Shader_Create)
{
  Reference<ShaderImpl> self = new ShaderImpl;
  self->program = ProgramObject_Load(args.vsPath, args.fsPath);
  DEBUG_ASSERT(self->program);
  return self;
}

Shader Shader_Create(const String& vs, const String& fs, const String& vsHeader, const String& fsHeader)
{
  Reference<ShaderImpl> self = new ShaderImpl;
  String vertCode = Location_Shader("vertex/" + vs)->ReadAscii();

  if (vertCode.empty())
  {
    Log_Error("Failed to load shader <" + vs + ">");
    return nullptr;
  }
  vertCode = vsHeader + '\n' + vertCode;

  String fragCode = Location_Shader("fragment/" + fs)->ReadAscii();

  if (fragCode.empty())
  {
    Log_Error("Could to load shader <" + fs + ">");
    return nullptr;
  }
  fragCode = fsHeader + '\n' + fragCode;

  if (!self->Create(vertCode, fragCode))
    Log_Error("Failed to create shader <" + vs + ", " + fs + ">");
  return self;
}

ShaderT* Shader_GetActive() { return gActiveShader; }

GL_Program Shader_GetCurrentProgram() { return gActiveProgram; }

DefineFunction(Shader_RecompileAll)
{
  for (auto it = GetShaderCache().begin(); it != GetShaderCache().end(); ++it)
  {
    const ShaderObject& shader = it->second;
    if (!shader->path.size())
      continue;

    String code = Location_Shader(shader->path)->ReadAscii();
    if (!code.size())
      continue;

    code = kVersionDirective + JSLPreprocess(code);
    if (code != shader->source)
    {
      shader->source = code;
      shader->Compile();
    }
  }

  for (auto it = GetProgramCache().begin(); it != GetProgramCache().end(); ++it)
  {
    const ProgramObject& program = it->second;
    int version = program->vertShader->version ^ program->fragShader->version;
    if (program->version != version)
      it->second->Link();
  }
}

void Shader_UseFixedFunction()
{
  if (gActiveProgram != GL_NullProgram)
  {
    gActiveProgram = GL_NullProgram;
    gActiveShader = nullptr;
    GL_UseProgram(GL_NullProgram);
  }
}
