#pragma once

#include "common_types.h"
#include "FontRendererResult.h"
#include "LayoutResult.h"
#include "FontConfig.h"
#include "LayoutConfig.h"


namespace fontium {
    struct LayoutConfig;

    struct glyph {
        uint id;
        int placeX;
        int placeY;
        int placeW;
        int placeH;
        int offsetX;
        int offsetY;
        int advance;
        QMap<int32_t, int> kerning;
    };

    struct bitmap_font {
        Img *img;
        std::vector<glyph> glyphs;
        str image_file_name;
        str name;
        str family, style;
        int tex_width;
        int tex_height;
        RenderedMetrics metrics;
        float scale;
        const FontConfig *font_config;
        const LayoutConfig *layout_config;

        static
        bitmap_font from(Img *img, str file,
                         const LayoutResult &layoutResult,
                         FontRendererResult &rendered,
                         const FontConfig &fontConfig,
                         const LayoutConfig &layoutConfig) {
            bitmap_font result{};

            result.img = img;
            result.image_file_name = std::move(file);
            result.metrics = rendered.metrics;
            result.font_config = &fontConfig;
            result.layout_config = &layoutConfig;
            result.metrics.height += fontConfig.line_spacing;

            for (const LayoutChar &lc : layoutResult.placed) {
                glyph g;
                g.id = lc.symbol;
                g.placeX = lc.x;
                g.placeY = lc.y;
                g.placeW = lc.w;
                g.placeH = lc.h;

                const RenderedChar &rc = rendered.chars[g.id];

                g.offsetX = rc.offsetX - layoutConfig.offset_left;
                g.offsetY = rc.offsetY + layoutConfig.offset_top;
                g.advance = rc.advance + fontConfig.char_spacing;
                g.kerning = rc.kerning;

                result.glyphs.push_back(g);
            }
            result.tex_width = layoutResult.width;
            result.tex_height = layoutResult.height;

            return result;
        }

    };

}