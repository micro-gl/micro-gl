#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common_types.h"
#include "FontRendererResult.h"
#include "ImageComposer.h"
#include "utils.h"


namespace fontium {
    struct FontConfig;

    class FontRenderer {
    public:
        explicit FontRenderer(const bytearray *font, const FontConfig *config);

        ~FontRenderer();

        FT_Face face() const { return m_ft_face; }

        FontRendererResult render(float scale);

    private:
        const FontConfig *m_config;
        FT_Library m_ft_library;
        FT_Face m_ft_face;
        const bytearray *_font;

        void applySize();

        void selectFace();

        RenderedChar copy_current_glyph(uint symbol);

        void append_kerning_to_char(RenderedChar &rendered_char, const int32_t *other, int amount);

        float m_scale;
    };

}