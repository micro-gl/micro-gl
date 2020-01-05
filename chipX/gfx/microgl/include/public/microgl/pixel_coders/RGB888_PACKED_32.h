#pragma once

#include <microgl/PixelCoder.h>

namespace coder {


    class RGB888_PACKED_32 : public PixelCoder<uint32_t, RGB888_PACKED_32> {
    public:
        static const channel MAX = (1 << 8) - 1;
        // include these if you want to enable the float overoading
        using PixelCoder::encode;
        using PixelCoder::decode;

        static
        inline void encode(const color_t &input, uint32_t &output) {
            output = (input.r << 16) + (input.g << 8) + input.b;
        }

        static
        inline void decode(const uint32_t &input, color_t &output) {
            output.r = (input & 0xFF0000) >> 16;
            output.g = (input & 0x00FF00) >> 8;
            output.b = (input & 0x0000FF);
            output.a = 255;
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
        inline const char * format() {
            return "RGB888";
        }

    };

}
