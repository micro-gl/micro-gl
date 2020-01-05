#pragma once

#include <microgl/PixelCoder.h>

namespace coder {


    class RGB332_PACKED_8 : public PixelCoder<uint8_t, RGB332_PACKED_8> {
    public:

        static
        inline void encode(const color_t &input, uint8_t &output) {
            output= ((input.r & 0b00000111) << 5) + ((input.g & 0b00000111) << 2) + ((input.b & 0b00000011));
        }

        static
        inline void decode(const uint8_t &input, color_t &output) {

            output.r = (input & 0b11100000) >> 5;
            output.g = (input & 0b00011100) >> 2;
            output.b = (input & 0b00000011);

            update_channel_bit(output);
        };

        static
        channel red_bits() {
            return 3;
        }
        static
        channel green_bits() {
            return 3;
        }
        static
        channel blue_bits() {
            return 2;
        }
        static
        channel alpha_bits() {
            return 0;
        }

        static
        inline const char * format() {
            return "RGB332_PACKED_8";
        }

    };

}
