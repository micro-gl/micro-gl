#pragma once

#include <microgl/PixelCoder.h>
#include <microgl/vec3.h>

namespace microgl {
    namespace coder {

        class RGB888_ARRAY : public PixelCoder<vec3<uint8_t>, 8,8,8,0, RGB888_ARRAY> {
        public:
            using PixelCoder::decode;
            using PixelCoder::encode;

            inline void encode(const color_t &input, vec3<uint8_t> &output) const {
                output.x = input.r, output.y = input.g, output.z = input.b;
            }

            inline void decode(const vec3<uint8_t> &input, color_t &output) const {
                output.r = input.x, output.g = input.y, output.b = input.z, output.a = 255;
            };

        };
    }
}