#pragma once

#include <microgl/PixelCoder.h>

namespace coder {


    class RGBA1555_PACKED_16 : public PixelCoder<uint16_t, RGBA1555_PACKED_16> {
    public:

        static
        inline void encode(const color_t &input, uint16_t &output) {
            output= ((input.a & 0x1) << 15) + ((input.r & 0x1F) << 10) + ((input.g & 0x1F) << 5) + (input.b & 0x1F);
        }

        static
        inline void decode(const uint16_t &input, color_t &output) {

            output.a = (input & 0b1000000000000000) >> 15;
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
            return 1;
        }

        static
        inline const char * format() {
            return "RGBA1555_PACKED_16";
        }

    };

}
