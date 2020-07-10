#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        class RGB8 : public PixelCoder<uint8_t, RGB8> {
        public:

//            static
            inline void encode(const color_t &input, uint8_t &output) const {
                output = input.r;
            }

//            static
            inline void decode(const uint8_t &input, color_t &output) const {
                output.r = output.g = output.b = input;

                update_channel_bit(output);
            };

            static constexpr channel red_bits() { return 8; }
            static constexpr channel green_bits() { return 8; }
            static constexpr channel blue_bits() { return 8; }
            static constexpr channel alpha_bits() { return 0; }

            static
            inline const char *format() {
                return "RGB8";
            }

        };

    }
}