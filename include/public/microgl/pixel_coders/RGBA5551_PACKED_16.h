#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        class RGBA5551_PACKED_16 : public pixel_coder<uint16_t, rgba_t<5,5,5,1>, RGBA5551_PACKED_16> {
        public:
            using pixel_coder::decode;
            using pixel_coder::encode;

            inline void encode(const color_t &input, uint16_t &output) {
                output = ((input.r & 0x1F) << 11) + ((input.g & 0x1F) << 6) + ((input.b & 0x1F) << 1) + (input.a & 0x1);
            }

            inline void decode(const uint16_t &input, color_t &output) {
                output.r = (input & 0b1111100000000000) >> 11;
                output.g = (input & 0b0000011111000000) >> 6;
                output.b = (input & 0b0000000000111110) >> 1;
                output.a = (input & 0b0000000000000001);
            };

        };
    }
}