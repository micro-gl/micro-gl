#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        class RGB888_PACKED_32 : public pixel_coder<uint32_t, rgba_t<8,8,8,0>, RGB888_PACKED_32> {
        public:
            using pixel_coder::encode;
            using pixel_coder::decode;

            static inline void encode(const color_t &input, uint32_t &output) {
                output = (input.r << 16) + (input.g << 8) + input.b;
            }

            static inline void decode(const uint32_t &input, color_t &output) {
                output.r = (input & 0x00FF0000) >> 16;
                output.g = (input & 0x0000FF00) >> 8;
                output.b = (input & 0x000000FF);
            };

        };

    }
}