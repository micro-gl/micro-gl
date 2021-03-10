#pragma once

#include <microgl/pixel_coders/RGBA_PACKED.h>

namespace microgl {
    namespace coder {

        template<uint8_t bits, uint8_t alpha_bits = 0>
        struct GRAYSCALE {
        private:
            constexpr static uint8_t mask = (1u<<bits) - 1;
        public:
            using rgba = rgba_t<bits, bits, bits, alpha_bits>;
            using pixel = uint8_t;

            inline void encode(const color_t &input, pixel &output) const {
                output = input.r & mask;
            }

            inline void decode(const pixel &input, color_t &output) const {
                output.r = output.g = output.b = input;
                if(alpha_bits!=0) output.a = input;
            };

        };

    }
}