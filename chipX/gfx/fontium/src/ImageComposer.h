#pragma once

#include "common_types.h"
#include "FontRendererResult.h"
#include "LayoutResult.h"
#include <fontium/LayoutConfig.h>

namespace fontium {
    class ImageComposer {
    public:
        ImageComposer() = delete;

        static
        Img *compose(const LayoutResult &data,
                     const LayoutConfig &config,
                     FontRendererResult &rendered) {

            Img *final = new Img(data.width, data.height, 1);
            for (auto &lc : data.placed) {
                const auto symbol = lc.symbol;
                bool contains = rendered.chars.find(symbol) != rendered.chars.end();
                if (!contains) continue;

                const RenderedChar &ren = rendered.chars[symbol];
                int x = lc.x + config.offset_left;
                int y = lc.y + config.offset_top;
                final->copy(*ren.img, x, y);
            }
            return final;
        }

    };
}