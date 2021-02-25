#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        class RGB888_PACKED_32 : public PixelCoder<uint32_t, 8,8,8,0, RGB888_PACKED_32> {
        public:
            using PixelCoder::decode;
            using PixelCoder::encode;

            inline void encode(const color_t &input, uint32_t &output) const {
                output = (input.r << 16) + (input.g << 8) + input.b;
            }

            inline void decode(const uint32_t &input, color_t &output) const {
                output.r = (input & 0xFF0000) >> 16;
                output.g = (input & 0x00FF00) >> 8;
                output.b = (input & 0x0000FF);
            };

        };
    }
}