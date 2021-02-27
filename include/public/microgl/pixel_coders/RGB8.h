#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        class RGB8 : public PixelCoder<uint8_t, 8, 8, 8, 8, RGB8> {
        public:
            using PixelCoder::decode;
            using PixelCoder::encode;

            inline void encode(const color_t &input, uint8_t &output) const {
                output = input.r;
            }

            inline void decode(const uint8_t &input, color_t &output) const {
                output.r = output.g = output.b = output.a = input;
            };

        };
    }
}