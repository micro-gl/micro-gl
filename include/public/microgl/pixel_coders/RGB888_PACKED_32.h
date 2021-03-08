#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        struct RGB888_PACKED_32 {
            using pixel = uint32_t;
            using rgba = rgba_t<8,8,8,0>;

            static inline void encode(const color_t &input, pixel &output) {
                output = (input.r << 16) + (input.g << 8) + input.b;
            }

            static inline void decode(const pixel &input, color_t &output) {
                output.r = (input & 0x00FF0000) >> 16;
                output.g = (input & 0x0000FF00) >> 8;
                output.b = (input & 0x000000FF);
            };

        };

    }
}