#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        class RGBA4444_PACKED_16 : public PixelCoder<uint16_t, RGBA4444_PACKED_16> {
        public:

            static
            inline void encode(const color_t &input, uint16_t &output) {
                output =
                        ((input.r & 0b00001111) << 12) + ((input.g & 0b00001111) << 8) + ((input.b & 0b00001111) << 4) +
                        (input.a & 0b00001111);
            }

            static
            inline void decode(const uint16_t &input, color_t &output) {

                output.r = (input & 0b1111000000000000) >> 12;
                output.g = (input & 0b0000111100000000) >> 8;
                output.b = (input & 0b0000000011110000) >> 4;
                output.a = (input & 0b0000000000001111);

                update_channel_bit(output);
            };

            static
            channel red_bits() {
                return 4;
            }

            static
            channel green_bits() {
                return 4;
            }

            static
            channel blue_bits() {
                return 4;
            }

            static
            channel alpha_bits() {
                return 4;
            }

            static
            inline const char *format() {
                return "RGBA4444_PACKED_16";
            }

        };

    }
}