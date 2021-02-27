#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        class RGB332_PACKED_8 : public PixelCoder<uint8_t, 3,3,2,0,RGB332_PACKED_8> {
        public:
            using PixelCoder::decode;
            using PixelCoder::encode;

            inline void encode(const color_t &input, uint8_t &output) {
                output = ((input.r & 0b00000111) << 5) + ((input.g & 0b00000111) << 2) + ((input.b & 0b00000011));
            }

            inline void decode(const uint8_t &input, color_t &output) {
                output.r = (input & 0b11100000) >> 5;
                output.g = (input & 0b00011100) >> 2;
                output.b = (input & 0b00000011);
            };

        };
    }
}