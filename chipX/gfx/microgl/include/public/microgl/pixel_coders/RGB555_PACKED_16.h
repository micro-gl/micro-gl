#pragma once

#include <microgl/PixelCoder.h>

namespace coder {


    class RGB555_PACKED_16 : public PixelCoder<uint16_t, RGB555_PACKED_16> {
    public:

        static
        inline void encode(const color_t &input, uint16_t &output) {
            output=  ((input.r & 0b00011111) << 10) + ((input.g & 0b00011111) << 5) + ((input.b & 0b00011111));
        }

        static
        inline void decode(const uint16_t &input, color_t &output) {

            output.r = (input & 0b0111110000000000) >> 10;
            output.g = (input & 0b0000001111100000) >> 5;
            output.b = (input & 0b0000000000011111);

            update_channel_bit(output);
        };

        static
        channel red_bits() {
            return 5;
        }
        static
        channel green_bits() {
            return 5;
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
            return "RGB555_PACKED_16";
        }

    };

}
