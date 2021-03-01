#pragma once

#include <microgl/PixelCoder.h>
#include <microgl/vec4.h>

namespace microgl {
    namespace coder {

        class RGBA8888_ARRAY : public PixelCoder<vec4<channel>, 8,8,8,8, RGBA8888_ARRAY> {
        public:
            using PixelCoder::decode;
            using PixelCoder::encode;

            inline void encode(const color_t &input, vec4<channel> &output) const {
                output.x = input.r, output.y = input.g, output.z = input.b; output.w = input.a;
            }

            inline void decode(const vec4<channel> &input, color_t &output) const {
                output.r = input.x, output.g = input.y, output.b = input.z, output.a = input.w;
            };

        };
    }
}