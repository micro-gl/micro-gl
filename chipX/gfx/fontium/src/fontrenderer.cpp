
#include "fontrenderer.h"
#include "fontconfig.h"

#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H

#include <math.h>

FontRenderer::FontRenderer(const bytearray * font, const FontConfig* config) : m_config(config)
{
    m_ft_library = 0;
    m_ft_face = 0;
    m_scale = 1.0f;
    _font = font;
    int error =  FT_Init_FreeType(&m_ft_library);
    if (error) {
        std::cerr << "FT_Init_FreeType error " << error;
        m_ft_library = 0;
    }
}


FontRenderer::~FontRenderer() {
    if (m_ft_face)
        FT_Done_Face(m_ft_face);
    if (m_ft_library)
        FT_Done_FreeType(m_ft_library);
}

RenderedChar FontRenderer::copy_current_glyph(uint symbol) {
    const FT_GlyphSlot  slot = m_ft_face->glyph;
    const FT_Bitmap* bm = &(slot->bitmap);
    int w = bm->width;
    int h = bm->rows;
    Img * img = new Img(w, h, 4);
    const uchar* src = bm->buffer;
    if (bm->pixel_mode==FT_PIXEL_MODE_GRAY) {
        for (int row=0;row<h;row++, src+=bm->pitch) {
            for (int col=0;col<w;col++) {
                 {
                     uint index = row*w + col;
                     uchar s = src[col];
                     (*img)[index][0] = s;
                }
            }
        }
    }else if (bm->pixel_mode==FT_PIXEL_MODE_MONO) {
        for (int row=0; row<h; row++, src+=bm->pitch) {
            uint index = row*w;
            auto * row_start = (*img)[index];
            for (int col=0;col<w/8;col++) { // unroll segments
                uchar s = src[col];
                (*row_start++) = (s&(1<<7))?255:0;
                (*row_start++) = (s&(1<<6))?255:0;
                (*row_start++) = (s&(1<<5))?255:0;
                (*row_start++) = (s&(1<<4))?255:0;
                (*row_start++) = (s&(1<<3))?255:0;
                (*row_start++) = (s&(1<<3))?255:0;
                (*row_start++) = (s&(1<<2))?255:0;
                (*row_start++) = (s&(1<<0))?255:0;
            }
            { // handle residual
                uchar s = src[w/8];
                uint index = (row*w)/8;
                int num = 7;
                switch (w%8) {
                case 7:  (*img)[index][0] = (s&(1<<(num--)))?255:0;
                case 6:  (*row_start++) = (s&(1<<(num--)))?255:0;
                case 5:  (*row_start++) = (s&(1<<(num--)))?255:0;
                case 4:  (*row_start++) = (s&(1<<(num--)))?255:0;
                case 3:  (*row_start++) = (s&(1<<(num--)))?255:0;
                case 2:  (*row_start++) = (s&(1<<(num--)))?255:0;
                case 1:  (*row_start++) = (s&(1<<(num--)))?255:0;
                case 0:
                    break;
                }
            }

        }

    }

    return RenderedChar(symbol, slot->bitmap_left, slot->bitmap_top, w, h, slot->advance.x/64, img);
//    m_rendered.chars[symbol]=RenderedChar(symbol, slot->bitmap_left, slot->bitmap_top, slot->advance.x/64, img);
//    m_chars.push_back(LayoutChar(symbol,slot->bitmap_left,-slot->bitmap_top,w,h));
//
//    return true;
}

void FontRenderer::append_kerning_to_char(RenderedChar & rendered_char, const uint* other, int amount) {
     FT_Vector  kerning;
     int32_t symbol= rendered_char.symbol;
     FT_UInt left =  FT_Get_Char_Index( m_ft_face, symbol );
    for (int i=0;i<amount;i++) {
        if (other[i]!=symbol) {
            FT_UInt right =  FT_Get_Char_Index( m_ft_face, other[i] );
            int error = FT_Get_Kerning( m_ft_face,          /* handle to face object */
                                      left,          /* left glyph index      */
                                      right,         /* right glyph index     */
                                      FT_KERNING_DEFAULT,  /* kerning mode          */
                                      &kerning );    /* target vector         */
            if (error) {

            } else {
                int advance = kerning.x / 64;
                if (advance!=0)
                    rendered_char.kerning[other[i]]=advance;
            }
        }
    }
}

void FontRenderer::selectFace() {
    if (m_ft_face) {
        FT_Done_Face(m_ft_face);
        m_ft_face = 0;
    }
    if (!m_ft_library) return;
    int error =  FT_New_Memory_Face(
            m_ft_library,
            reinterpret_cast<const FT_Byte*>(_font.data()), _font.size(),
            m_config->face_index, &m_ft_face);
    if (error) {
        std::cerr << "FT_New_Memory_Face error " << error;
    } else {
        error = FT_Select_Charmap(
                    m_ft_face,               /* target face object */
                    FT_ENCODING_UNICODE );
        if (error) {
            std::cerr << "FT_Select_CharMap error " << error;
        }
    }
}


void FontRenderer::applySize() {
    if (!m_ft_face) return;
    bool fixedsize = (FT_FACE_FLAG_SCALABLE & m_ft_face->face_flags ) == 0;
    int size = m_config->size;
    if (fixedsize) {
        // todo:: return here for pcf/bdf
        std::cerr << "fixed size not impemented";
    } else {
        int size_x = static_cast<int>(m_config->width * size * 64.0f / 100.0f);
        int size_y = static_cast<int>(m_config->height * size * 64.0f / 100.0f);
        int error = FT_Set_Char_Size(m_ft_face,
                                     FT_F26Dot6(size_x),
                                     FT_F26Dot6(size_y), m_config->dpi * m_scale,
                                     m_config->dpi * m_scale);
        //int error = FT_Set_Pixel_Sizes(m_ft_face,size_x/64,size_y/64);
        if (error) {
            std::cerr << "FT_Set_Char_Size error " << error;
        }
    }
}

FontRendererResult FontRenderer::render(float scale) {
    m_scale = scale;
    selectFace();
    applySize();

    if (!m_ft_face)
        return {};

    FontRendererResult result{};
    std::cout << " begin rasterize_font ";

    if (m_config->italic != 0) {
        FT_Matrix matrix;
        const float angle = (-M_PI*m_config->italic) / 180.0f;
        matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
        matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
        matrix.yx = (FT_Fixed)( 0/*sin( angle )*/ * 0x10000L );
        matrix.yy = (FT_Fixed)( 1/*cos( angle )*/ * 0x10000L );
        FT_Set_Transform(m_ft_face,&matrix,0);
    } else {
        FT_Set_Transform(m_ft_face,0,0);
    }

    /// fill metrics
    if (FT_IS_SCALABLE(m_ft_face)) {
        result.metrics.ascender = m_ft_face->size->metrics.ascender / 64;
        result.metrics.descender = m_ft_face->size->metrics.descender/ 64;
        result.metrics.height = m_ft_face->size->metrics.height/ 64;

    } else {
        result.metrics.ascender = m_ft_face->ascender;
        result.metrics.descender = m_ft_face->descender;
        result.metrics.height = m_ft_face->height;
    }


    bool use_kerning = FT_HAS_KERNING( m_ft_face );

//    QVector<uint> ucs4chars = m_config->characters().toUcs4();
    auto ucs4chars = utf_8_to_32(m_config->characters);
    ucs4chars.push_back(0);
    int error = 0;
    for (int i=0;i+1<ucs4chars.size();i++) {
        int symbol = ucs4chars[i];
        int glyph_index = FT_Get_Char_Index(m_ft_face, symbol);
        if (glyph_index == 0 && !m_config->render_missing)
            continue;

        FT_Int32 flags = FT_LOAD_DEFAULT;
        if (!(m_config->antialiased)) {
            flags = flags | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;
        } else {
            switch (m_config->antialiased) {
                case FontConfig::AAliasingNormal:
                    flags |= FT_LOAD_TARGET_NORMAL;
                    break;
                case FontConfig::AAliasingLight:
                    flags |= FT_LOAD_TARGET_LIGHT;
                    break;
                case FontConfig::AAliasingLCDhor:
                    flags |= FT_LOAD_TARGET_LCD;
                    break;
                case FontConfig::AAliasingLCDvert:
                    flags |= FT_LOAD_TARGET_LCD_V;
                    break;
                default:
                    break;
            }
        }
        switch (m_config->hinting) {
            case FontConfig::HintingDisable:
                flags = flags | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
                break;
            case FontConfig::HintingForceFreetypeAuto:
                flags = flags | FT_LOAD_FORCE_AUTOHINT;
                break;
            case FontConfig::HintingDisableFreetypeAuto:
                flags = flags | FT_LOAD_NO_AUTOHINT;
                break;
            default:
                break;
        }

        error = FT_Load_Glyph(m_ft_face, glyph_index, flags);
        if (error)
            continue;
        if (m_config->bold != 0) {
            FT_Pos strength = m_config->size * m_config->bold;
            if (m_ft_face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                FT_Outline_Embolden(&m_ft_face->glyph->outline, strength);
        }
        if (m_ft_face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
            error = FT_Render_Glyph(m_ft_face->glyph,
                                    m_config->antialiased ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO);
        }
        if (error) continue;
        RenderedChar rendered = copy_current_glyph(symbol);

        if (use_kerning) {
            append_kerning_to_char(rendered,
//                        reinterpret_cast< uint *>(&ucs4chars.front()),
                                   reinterpret_cast<const uint *>(ucs4chars.data()),
                                   ucs4chars.size() - 1);
        }

        result.chars[symbol] = rendered;
    }

    return result;
}