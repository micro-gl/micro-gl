#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        class RGBA4444_PACKED_16 : public pixel_coder<uint16_t, rgba_t<4,4,4,4>, RGBA4444_PACKED_16> {
        public:
            using pixel_coder::decode;
            using pixel_coder::encode;

            inline void encode(const color_t &input, uint16_t &output) {
                output =
                        ((input.r & 0b00001111) << 12) + ((input.g & 0b00001111) << 8) + ((input.b & 0b00001111) << 4) +
                        (input.a & 0b00001111);
            }

            inline void decode(const uint16_t &input, color_t &output) {
                output.r = (input & 0b1111000000000000) >> 12;
                output.g = (input & 0b0000111100000000) >> 8;
                output.b = (input & 0b0000000011110000) >> 4;
                output.a = (input & 0b0000000000001111);
            };

        };
    }
}