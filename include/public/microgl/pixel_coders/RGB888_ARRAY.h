#pragma once

#include <microgl/pixel_coder.h>
#include <microgl/vec3.h>

namespace microgl {
    namespace coder {

        class RGB888_ARRAY : public pixel_coder<vec3<uint8_t>, rgba_t<8,8,8,0>, RGB888_ARRAY> {
        public:
            using pixel_coder::encode;
            using pixel_coder::decode;

            static inline void encode(const color_t &input, vec3<uint8_t> &output) {
                output.x = input.r, output.y = input.g, output.z = input.b;
            }

            static inline void decode(const vec3<uint8_t> &input, color_t &output) {
                output.r = input.x, output.g = input.y, output.b = input.z, output.a = 255;
            };

        };

    }

}