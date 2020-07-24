#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common_types.h"
#include "FontRendererResult.h"
#include "utils.h"

namespace fontium {
    struct FontConfig;

    class FontRenderer {
    public:
        FontRenderer(const bytearray &font, int faceIndex=0);
        ~FontRenderer();

        FontRendererResult render(const FontConfig &config, float scale=1.0f);
        FT_Face face() const { return m_ft_face; }

    private:
        FT_Library m_ft_library;
        FT_Face m_ft_face;

        RenderedChar copy_current_glyph(uint symbol);
        void append_kerning_to_char(RenderedChar &rendered_char, const int32_t *other, int amount);
        void throw_exception(const std::string & what, int code);
    };

}