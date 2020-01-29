#pragma once

#include <microgl/PixelCoder.h>
#include <microgl/vec4.h>

namespace coder {

    class RGBA8888_ARRAY : public PixelCoder<vec4<channel>, RGBA8888_ARRAY> {
    public:

        static
        inline void encode(const color_t &input, vec4<channel> &output) {
            output.x = input.r, output.y = input.g, output.z = input.b;, output.w = input.a;
        }

        static
        inline void decode(const vec4<channel> &input, color_t &output) {
            output.r = input.x, output.g = input.y, output.b = input.z, output.a = input.w;
            update_channel_bit(output);
        };

        static
        channel red_bits() {
            return 8;
        }
        static
        channel green_bits() {
            return 8;
        }
        static
        channel blue_bits() {
            return 8;
        }
        static
        channel alpha_bits() {
            return 8;
        }
        static
        inline const char* format() {
            return "RGBA8888_ARRAY";
        }

    };

}
