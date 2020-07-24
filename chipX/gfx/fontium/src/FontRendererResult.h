#pragma once

#include "common_types.h"

namespace fontium {
    struct RenderedChar {
        int32_t symbol=0;
        int offsetX=0, offsetY=0, w=0, h=0, advance=0;
        Img *img= nullptr;
        std::map<int32_t, int> kerning;

        RenderedChar()= default;
        RenderedChar(int32_t symbol, int x, int y, int w, int h, int a, Img *img) :
                symbol(symbol), offsetX(x), offsetY(y), w{w}, h{h}, advance(a), img(img) {}

        ~RenderedChar() = default;
    };

    struct RenderedMetrics {
        int ascender, descender, height;
    };

    struct FontRendererResult {
        std::map<int32_t, RenderedChar> chars;
        RenderedMetrics metrics;
        str family, style;

        void dispose() {
            for (auto & entry : chars) {
                delete entry.second.img;
            }
        }
    };
}