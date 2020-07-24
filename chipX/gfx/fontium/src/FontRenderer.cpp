#include <stdexcept>
#include "FontRenderer.h"
#include <fontium/FontConfig.h>

#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H

#include <cmath>

namespace fontium {
    void FontRenderer::throw_exception(const std::string & which, int code) {
        str text = "freetype - " + which + ", error code " + std::to_string(code);
        throw std::runtime_error(text);
    }

    FontRenderer::FontRenderer(const bytearray &font, int faceIndex) {
        m_ft_library = nullptr;
        m_ft_face = nullptr;

        int error = FT_Init_FreeType(&m_ft_library);
        if (error) throw_exception("FT_Init_FreeType", error);
        error = FT_New_Memory_Face(
                m_ft_library,
                reinterpret_cast<const FT_Byte *>(font.data()), font.size(),
                faceIndex, &m_ft_face);
        if (error) 
            throw_exception("FT_New_Memory_Face", error);
        
        error = FT_Select_Charmap(m_ft_face, FT_ENCODING_UNICODE);
        if (error) 
            throw_exception("FT_Select_CharMap", error);
    }

    FontRenderer::~FontRenderer() {
        if (m_ft_face)
            FT_Done_Face(m_ft_face);
        if (m_ft_library)
            FT_Done_FreeType(m_ft_library);
    }

    RenderedChar FontRenderer::copy_current_glyph(uint symbol) {
        const FT_GlyphSlot slot = m_ft_face->glyph;
        const FT_Bitmap *bm = &(slot->bitmap);
        uint w = bm->width;
        uint h = bm->rows;
        Img *img = new Img(w, h, 1);
        const uchar *src = bm->buffer;
        if (bm->pixel_mode == FT_PIXEL_MODE_GRAY) {
            for (uint row = 0; row < h; row++, src += bm->pitch) {
                for (uint col = 0; col < w; col++) {
                    {
                        uint index = row * w + col;
                        uchar s = src[col];
                        (*img)[index][0] = s;
                    }
                }
            }
        } else if (bm->pixel_mode == FT_PIXEL_MODE_MONO) {
            uint row_w=w/8;
            for (uint row = 0; row < h; row++, src+=bm->pitch) {
                uint index = row * w;
                auto *row_start = (*img)[index];
                for (uint col = 0; col < row_w; col++) { // unroll segments
                    uint s = src[col];
                    (*row_start++) = (s & (uint(1) << 7)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 6)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 5)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 4)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 3)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 2)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 1)) ? 255 : 0;
                    (*row_start++) = (s & (uint(1) << 0)) ? 255 : 0;
                }
                {   // handle residual and padding
                    uchar s = src[w / 8];
                    int num = 7;
                    switch (w % 8) {
                        case 7: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                        case 6: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                        case 5: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                        case 4: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                        case 3: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                        case 2: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                        case 1: (*row_start++) = (s & (1 << (num--))) ? 255 : 0;
                    }
                }
            }

        }

        return RenderedChar(symbol, slot->bitmap_left, slot->bitmap_top, w, h, slot->advance.x / 64, img);
    }

    void FontRenderer::append_kerning_to_char(RenderedChar &rendered_char, const int32_t *other, int amount) {
        FT_Vector kerning;
        int32_t symbol = rendered_char.symbol;
        FT_UInt left = FT_Get_Char_Index(m_ft_face, symbol);
        for (int i = 0; i < amount; i++) {
            if (other[i] != symbol) {
                FT_UInt right = FT_Get_Char_Index(m_ft_face, other[i]);
                int error = FT_Get_Kerning(m_ft_face,          /* handle to face object */
                                           left,          /* left glyph index      */
                                           right,         /* right glyph index     */
                                           FT_KERNING_DEFAULT,  /* kerning mode          */
                                           &kerning);    /* target vector         */
                if (error) {

                } else {
                    int advance = kerning.x / 64;
                    if (advance != 0)
                        rendered_char.kerning[other[i]] = advance;
                }
            }
        }
    }

    FontRendererResult FontRenderer::render(const FontConfig &config, float scale) {

        if (!m_ft_face)
            throw std::runtime_error("FontRenderer - tried to render a face that was not loaded");

        // apply size
        const bool fixedsize = (FT_FACE_FLAG_SCALABLE & m_ft_face->face_flags)==0;
        if (!fixedsize) {
            int size_x = static_cast<int>(config.scale_width * config.size * 64.0f / 100.0f);
            int size_y = static_cast<int>(config.scale_height * config.size * 64.0f / 100.0f);
            int error = FT_Set_Char_Size(m_ft_face,
                                         FT_F26Dot6(size_x),
                                         FT_F26Dot6(size_y), config.dpi * scale,
                                         config.dpi * scale);
            //int error = FT_Set_Pixel_Sizes(m_ft_face,size_x/64,size_y/64);
            if (error) throw_exception("FT_Set_Char_Size", error);
        }

        FontRendererResult result{};
        std::cout << "    rasterizing font ";

        // apply italic
        if (config.italic != 0) {
            FT_Matrix matrix;
            const float angle = (-M_PI * config.italic) / 180.0f;
            matrix.xx = (FT_Fixed) (cos(angle) * 0x10000L);
            matrix.xy = (FT_Fixed) (-sin(angle) * 0x10000L);
            matrix.yx = (FT_Fixed) (0/*sin( angle )*/ * 0x10000L);
            matrix.yy = (FT_Fixed) (1/*cos( angle )*/ * 0x10000L);
            FT_Set_Transform(m_ft_face, &matrix, 0);
        } else {
            FT_Set_Transform(m_ft_face, 0, 0);
        }

        /// fill metrics
        if (FT_IS_SCALABLE(m_ft_face)) {
            result.metrics.ascender = m_ft_face->size->metrics.ascender / 64;
            result.metrics.descender = m_ft_face->size->metrics.descender / 64;
            result.metrics.height = m_ft_face->size->metrics.height / 64;

        } else {
            result.metrics.ascender = m_ft_face->ascender;
            result.metrics.descender = m_ft_face->descender;
            result.metrics.height = m_ft_face->height;
        }

        // start rasterizing glyphs
        bool use_kerning = FT_HAS_KERNING(m_ft_face);
        auto ucs4chars = utf_8_to_32(config.characters);
        ucs4chars.push_back(0);
        int error = 0;
        bool antialiased=config.antialiasing!=Antialiasing::None;
        for (unsigned i = 0; i + 1 < ucs4chars.size(); i++) {
            int symbol = ucs4chars[i];
            int glyph_index = FT_Get_Char_Index(m_ft_face, symbol);
            if (glyph_index == 0 && !config.render_missing)
                continue;

            FT_Int32 flags = FT_LOAD_DEFAULT;
            if (!antialiased) {
                flags = flags | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;
            } else {
                switch (config.antialiasing) {
                    case Antialiasing::Normal:
                        flags |= FT_LOAD_TARGET_NORMAL;
                        break;
                    case Antialiasing::Light:
                        flags |= FT_LOAD_TARGET_LIGHT;
                        break;
                    case Antialiasing::LCDH:
                        flags |= FT_LOAD_TARGET_LCD;
                        break;
                    case Antialiasing::LCDV:
                        flags |= FT_LOAD_TARGET_LCD_V;
                        break;
                    default:
                        break;
                }
            }
            switch (config.hinting) {
                case Hinting::Disable:
                    flags = flags | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
                    break;
                case Hinting::ForceFreetypeAuto:
                    flags = flags | FT_LOAD_FORCE_AUTOHINT;
                    break;
                case Hinting::DisableFreetypeAuto:
                    flags = flags | FT_LOAD_NO_AUTOHINT;
                    break;
                default:
                    flags = flags | FT_LOAD_DEFAULT;
                    break;
            }

            error = FT_Load_Glyph(m_ft_face, glyph_index, flags);
            if (error)
                continue;
            if (config.bold != 0) {
                FT_Pos strength = config.size * config.bold;
                if (m_ft_face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                    FT_Outline_Embolden(&m_ft_face->glyph->outline, strength);
            }
            if (m_ft_face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
                error = FT_Render_Glyph(m_ft_face->glyph,
                                        antialiased ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO);
            }
            if (error) continue;
            RenderedChar rendered = copy_current_glyph(symbol);

            if (use_kerning) {
                append_kerning_to_char(rendered,
                                       reinterpret_cast<const int32_t *>(ucs4chars.data()),
                                       ucs4chars.size() - 1);
            }

            result.chars[symbol] = rendered;
        }
        result.family= std::string(face()->family_name);
        result.style= std::string(face()->style_name);

        return result;
    }


}