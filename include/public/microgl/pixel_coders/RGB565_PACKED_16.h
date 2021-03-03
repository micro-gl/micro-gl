#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        class RGB565_PACKED_16 : public pixel_coder<uint16_t, rgba_t<5,6,5,0>, RGB565_PACKED_16> {
        public:
            using pixel_coder::decode;
            using pixel_coder::encode;

            inline void encode(const color_t &input, uint16_t &output) {
                output = ((input.r & 0x1F) << 11) + ((input.g & 0x3F) << 5) + (input.b & 0x1F);
            }

            inline void decode(const uint16_t &input, color_t &output) {
                output.r = (input & 0xF800) >> 11;         // rrrrr... ........ -> 000rrrrr
                output.g = (input & 0x07E0) >> 5;          // .....ggg ggg..... -> 00gggggg
                output.b = (input & 0x1F);                 // ............bbbbb -> 000bbbbb
            };

        };
    }
}