#pragma once

#include <microgl/PixelCoder.h>

namespace coder {


    class RGB565_PACKED_16 : public PixelCoder<uint16_t, RGB565_PACKED_16> {
    public:

        static
        inline void encode(const color_t &input, uint16_t &output) {
            output=  ((input.r & 0x1F) << 11) + ((input.g & 0x3F) << 5) + (input.b & 0x1F);
        }

        static
        inline void decode(const uint16_t &input, color_t &output) {

            output.r = (input & 0xF800) >> 11;         // rrrrr... ........ -> 000rrrrr
            output.g = (input & 0x07E0) >> 5;          // .....ggg ggg..... -> 00gggggg
            output.b = (input & 0x1F);                 // ............bbbbb -> 000bbbbb
            output.a = 255;

            update_channel_bit(output);
        };

        static
        channel red_bits() {
            return 5;
        }
        static
        channel green_bits() {
            return 6;
        }
        static
        channel blue_bits() {
            return 5;
        }
        static
        channel alpha_bits() {
            return 0;
        }

        static
        inline const char * format() {
            return "RGB565_PACKED_16";
        }

    };

}
