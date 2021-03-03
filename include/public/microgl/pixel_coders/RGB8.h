#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        class RGB8 : public pixel_coder<uint8_t, rgba_t<8, 8, 8, 0>, RGB8> {
        public:
            using pixel_coder::decode;
            using pixel_coder::encode;

            inline void encode(const color_t &input, uint8_t &output) const {
                output = input.r;
            }

            inline void decode(const uint8_t &input, color_t &output) const {
                output.r = output.g = output.b = input;
            };

        };
    }
}