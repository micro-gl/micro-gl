#pragma once

#include <libs/freetype-2.10.0/src/psaux/pstypes.h>
#include "common_types.h"
#include "FontRendererResult.h"
#include "LayoutResult.h"
#include "fontconfig.h"
#include "layoutconfig.h"

struct FontConfig;
struct LayoutConfig;
struct FontRendererResult;
struct LayoutResult;

struct Symbol {
    uint id;
    int placeX;
    int placeY;
    int placeW;
    int placeH;
    int offsetX;
    int offsetY;
    int advance;
    QMap<int32_t,int> kerning;
};

struct bitmap_font {
    str texture_file;
    int tex_width;
    int tex_height;
    const FontConfig* font_config;
    const LayoutConfig* layout_config;
    RenderedMetrics metrics;
    FT_Face face;
    float scale;
    vector<Symbol> chars;

    static
    bitmap_font from(str file,
                     const LayoutResult* layoutResult,
                     FontRendererResult* rendered,
                     const FontConfig * fontConfig,
                     const LayoutConfig * layoutConfig) {
        bitmap_font result{};

        result.texture_file = std::move(file);
        result.metrics = rendered->metrics;
        result.font_config = fontConfig;
        result.layout_config = layoutConfig;
        result.metrics.height+=fontConfig->line_spacing;

        for ( const LayoutChar& lc : layoutResult->placed) {
            Symbol symb;
            symb.id = lc.symbol;
            symb.placeX = lc.x;
            symb.placeY = lc.y;
            symb.placeW = lc.w;
            symb.placeH = lc.h;

            const RenderedChar& rc = rendered->chars[symb.id];

            symb.offsetX = rc.offsetX-layoutConfig->offset_left;
            symb.offsetY = rc.offsetY+layoutConfig->offset_top;
            symb.advance = rc.advance + fontConfig->char_spacing;
            symb.kerning = rc.kerning;

            result.chars.push_back(symb);
        }
        result.tex_width = layoutResult->width;
        result.tex_height = layoutResult->height;

        return result;
    }

};