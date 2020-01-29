#pragma once

#include <microgl/PixelCoder.h>
#include <microgl/vec3.h>

namespace coder {

    class RGB888_ARRAY : public PixelCoder<vec3<uint8_t>, RGB888_ARRAY> {
    public:

        static
        inline void encode(const color_t &input, vec3<uint8_t> &output) {
            output.x = input.r, output.y = input.g, output.z = input.b;
        }

        static
        inline void decode(const vec3<uint8_t> &input, color_t &output) {
            output.r = input.x, output.g = input.y, output.b = input.z, output.a = 255;
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
            return 0;
        }
        static
        inline const char* format() {
            return "RGB888_ARRAY";
        }

    };

}
