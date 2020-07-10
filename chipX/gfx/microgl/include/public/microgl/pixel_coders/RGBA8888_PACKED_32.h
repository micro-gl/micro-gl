#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        class RGBA8888_PACKED_32 : public PixelCoder<uint32_t, 8,8,8,8, RGBA8888_PACKED_32> {
        public:

            inline void encode(const color_t &input, uint32_t &output) {
                output = (input.r << 24) + (input.g << 16) + (input.b << 8) + input.a;
            }

            inline void decode(const uint32_t &input, color_t &output) {
                output.r = (input & 0xFF000000) >> 24;
                output.g = (input & 0x00FF0000) >> 16;
                output.b = (input & 0x0000FF00) >> 8;
                output.a = (input & 0x000000FF);
            };

        };
    }
}