#include "Font.h"
#include "Array.h"
#include "Color.h"
#include "DrawState.h"
#include "Map.h"
#include "ProgramLog.h"
#include "Renderer.h"
#include "Shader.h"
#include "StackFrame.h"
#include "Texture2D.h"
#include "UniString.h"
#include "Viewport.h"
#include "FileSys.h"
#include <FreeType/freetype.h>

const V2U kTextureSize = 1024;
const V2U kSDFSize = 1024;
constexpr int kFontSize = 64;
constexpr int kPadding = 8;

TypeAlias(Reference<FontT>, Font);

namespace
{
  using FontCache = Map<String, Font>;
  using KerningMap = std::unordered_map<uint32_t, float>;

  FontCache& GetFontCache()
  {
    static FontCache cache;
    return cache;
  }

  struct TextGlyph
  {
    uint32_t codepoint;
    V2 uvMin;
    V2 uvMax;
    V2 origin;
    V2 size;
    float advance;
    bool exists;
    mutable KerningMap kerning;
  };

  AutoClass(TextVertex, V3, p, V4, color, V3, uvScale)
    TextVertex() {}
  };

  struct FontImpl : FontT
  {
    mutable Map<uint32_t, TextGlyph> glyphs;
    mutable V2U cursor;
    Texture2D texture;
    Texture2D glyphBitmap;
    FT_Face face;
    FT_Library ft;

    FontImpl()
      : cursor(0) {}

    ~FontImpl() override
    {
      /* Clean up FT. */
      {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
      }
    }

    void AddGlyph(uint32_t codepoint) const
    {
      static Shader compute = Shader_Create("identity.jsl", "compute/sdffont.jsl");

      uint32_t glyphIndex = FT_Get_Char_Index(face, codepoint);
      if (glyphIndex == 0)
      {
        TextGlyph& glyph = glyphs[codepoint];
        glyph.codepoint = codepoint;
        glyph.exists = false;
        return;
      }

      if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) != 0) Fatal("FreeType -- Failed to load glyph");

      if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) Fatal("FreeType -- Failed to render glyph");

      /* Copy FT bitmap to our buffer. */
      const FT_Bitmap& bitmap = face->glyph->bitmap;
      uint sx = bitmap.width;
      uint sy = bitmap.rows;

      Array<float> buffer(sx * sy);
      const uchar* pixels = bitmap.buffer;
      for (uint y = 0; y < sy; ++y)
      {
        for (uint x = 0; x < sx; ++x)
        {
          // buffer[x + y * sx] = (pixels[x / 8] & (1 << (7 - x % 8))) ? 1 : 0;
          buffer[x + y * sx] = static_cast<float>(pixels[x]) / 255.0f;
        }
        pixels += bitmap.pitch;
      }

      uint px = cursor.x + kPadding + (kFontSize - sx) / 2;
      uint py = cursor.y + kPadding + (kFontSize - sy) / 2;

      if (cursor.x + (2 * kPadding + kFontSize) >= kTextureSize.x)
      {
        cursor.x = 0;
        cursor.y += 2 * kPadding + kFontSize;
        px = cursor.x + kPadding + (kFontSize - sx) / 2;
        py = cursor.y + kPadding + (kFontSize - sy) / 2;
      }

      /* Write glyph to bitmap. */
      glyphBitmap->SetData(px, py, sx, sy, GL_PixelFormat::Red, GL_DataFormat::Float, buffer.data());

      TextGlyph& glyph = glyphs[codepoint];
      glyph.codepoint = codepoint;
      glyph.uvMin = static_cast<V2>(cursor) / static_cast<V2>(kTextureSize);
      glyph.uvMax = (static_cast<V2>(cursor) + V2(2 * kPadding + kFontSize)) / static_cast<V2>(kTextureSize);
      glyph.origin = V2(px, py) - V2(cursor) + V2(-face->glyph->bitmap_left, face->glyph->bitmap_top);
      glyph.advance = static_cast<float>(face->glyph->advance.x >> 6);
      glyph.size = V2(2 * kPadding + kFontSize);
      glyph.exists = true;

      /* Compute the distance field. */
      {
        // glyphBitmap->GenerateMipmap();
        Renderer_PushAllBuffers();
        texture->Bind(0);
        (*compute)("bitmap", glyphBitmap)("frame", kTextureSize)("radius", static_cast<float>(kFontSize));
        Renderer_SetShader(*compute);
        RendererState rs(BlendMode::Disabled, CullMode::Backface, false, false);
        Renderer_PushScissorOn(cursor, glyph.size);
        Renderer_DrawQuad();
        Renderer_PopScissor();
        texture->Unbind();
        texture->GenerateMipmap();
        Renderer_PopAllBuffers();
      }

      /* Advance the cursor. */
      cursor.x += 2 * kPadding + kFontSize;

      // texture->SaveTo("font.bmp");
    }

    void Create(const String& path)
    {
      /* Create the FT library. */
      {
        if (FT_Init_FreeType(&ft))
        {
          Log_Error("FreeType -- Failed to initialize");
          return;
        }
      }

      /* Create the FT face. */
      {
        String realPath = FileSys::GetHomeDirectoryA() + "Font\\" + String_Replace(path, '/', '\\');
        if (FT_New_Face(ft, realPath.c_str(), 0, &face))
        {
          Log_Error("FreeType -- Failed to load font " + realPath);
          return;
        }
      }

      FT_Set_Pixel_Sizes(face, kFontSize, kFontSize);

      /* Create and clear the maps. */
      {
        glyphBitmap = Texture_Create(kTextureSize.x, kTextureSize.y, GL_TextureFormat::R16F);

        glyphBitmap->Bind(0);
        Renderer_Clear(V4(0));
        glyphBitmap->Unbind();
        glyphBitmap->SetWrapMode(GL_TextureWrapMode::ClampToEdge);

        texture = Texture_Create(kSDFSize.x, kSDFSize.y, GL_TextureFormat::R32F);
      }

#if 0
      /* Render all chars to the bitmap. */
      {
        uint32_t glyphIndex;
        uint32_t c = FT_Get_First_Char(face, &glyphIndex);

        while (glyphIndex != 0)
        {
          AddGlyph(c);
          c = FT_Get_Next_Char(face, c, &glyphIndex);
        }
      }
#endif
    }

    void Draw(const String& text, const V2& position, float size, const Color& color, float alpha, bool additive) const override
    {
      AUTO_FRAME;
      size /= kFontSize;

      static Vector<TextVertex> vertices;
      static Vector<uint> indices;
      vertices.clear();
      indices.clear();

      V2 p = position;
      auto c = V4(color, alpha);

      const TextGlyph* prev = nullptr;
      for (UniStringIterator it = UniString_Begin(text); it.HasMore(); it.Advance())
      {
        uint32_t codepoint = it.Get();
        const TextGlyph* glyph = glyphs.get(codepoint);
        if (!glyph)
        {
          AddGlyph(codepoint);
          glyph = glyphs.get(codepoint);
        }

        if (!glyph->exists)
        {
          prev = nullptr;
          continue;
        }

        /* Indices. */
        {
          uint offset = vertices.size();
          indices.push(offset + 0);
          indices.push(offset + 1);
          indices.push(offset + 2);
          indices.push(offset + 0);
          indices.push(offset + 2);
          indices.push(offset + 3);
        }

        if (prev)
          p.x += size * GetKerning(prev, glyph);
        prev = glyph;

        /* Vertices. */
        {
          V2 p0 = p - size * glyph->origin;
          V2 p1 = p0 + size * V2(glyph->size.x, 0);
          V2 p2 = p0 + size * glyph->size;
          V2 p3 = p0 + size * V2(0, glyph->size.y);
          vertices.push(TextVertex(V3(p0, 0), c, V3(glyph->uvMin.x, glyph->uvMin.y, size)));
          vertices.push(TextVertex(V3(p1, 0), c, V3(glyph->uvMax.x, glyph->uvMin.y, size)));
          vertices.push(TextVertex(V3(p2, 0), c, V3(glyph->uvMax.x, glyph->uvMax.y, size)));
          vertices.push(TextVertex(V3(p3, 0), c, V3(glyph->uvMin.x, glyph->uvMax.y, size)));
        }

        p.x += size * glyph->advance;
      }

      if (indices.empty())
        return;

      /* Render. */
      {
        static Shader shader = Shader_Create("widget.jsl", "ui/text.jsl");
        shader->BindInput(1, "vert_attrib1");
        shader->BindInput(2, "vert_attrib2");
        shader->BindInput(3, "vert_attrib3");
        shader->BindInput(4, "vert_attrib4");

        DrawState_Link(shader);
        (*shader)("additive", additive ? 1 : 0)("font", texture)("frame", V2(kSDFSize));

        Renderer_SetShader(*shader);
        Renderer_DrawVertices(vertices.data(), Type_Get<TextVertex>(), indices.data(), indices.size(), GL_IndexFormat::Int);
      }
    }

    /* Kerning caching. */
    float GetKerning(const TextGlyph* prev, const TextGlyph* curr) const
    {
      KerningMap::const_iterator it = prev->kerning.find(curr->codepoint);
      if (it != prev->kerning.end())
        return it->second;

      FT_Vector kern;
      FT_Get_Kerning(face, FT_Get_Char_Index(face, prev->codepoint), FT_Get_Char_Index(face, curr->codepoint), FT_KERNING_DEFAULT, &kern);

      float kerning = kern.x >> 6;
      prev->kerning[curr->codepoint] = kerning;
      return kerning;
    }

    V2 GetTextSize(const String& text, float size) const override
    {
      float sx = 0;
      float sy = size;
      size /= kFontSize;

      const TextGlyph* prev = nullptr;
      for (UniStringIterator it = UniString_Begin(text); it.HasMore(); it.Advance())
      {
        uint32_t codepoint = it.Get();
        const TextGlyph* glyph = glyphs.get(codepoint);
        if (!glyph)
        {
          AddGlyph(codepoint);
          glyph = glyphs.get(codepoint);
        }

        if (!glyph->exists)
        {
          prev = nullptr;
          continue;
        }

        if (prev)
          sx += size * GetKerning(prev, glyph);
        prev = glyph;
        sx += size * glyph->advance;
      }

      return V2(sx, sy);
    }
  };
}

DefineFunction(Font_Get)
{
  Font& font = GetFontCache()[args.path];
  if (!font)
  {
    Reference<FontImpl> self = new FontImpl;
    self->Create(args.path);
    font = self.get();
  }
  return font;
}
