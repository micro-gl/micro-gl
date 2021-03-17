#pragma once

#include <microgl/lut_bits.h>

namespace microgl {
    namespace coder {

        /**
         * map single channel with BPP bits to Alpha channel and
         * keep other channels maximum intensity. This is good for bitmap fonts
         *
         * @tparam BPP bits of channel
         * @tparam rgba_ rgba info
         */
        template <channel BPP, typename rgba_>
        struct BPP_RGBA {
            using rgba = rgba_;
            using pixel = uint8_t;
        private:
            // dynamic look-up tables
            static constexpr uint8_t full_r = (1u<<rgba::r) - 1;
            static constexpr uint8_t full_g = (1u<<rgba::g) - 1;
            static constexpr uint8_t full_b = (1u<<rgba::b) - 1;
            static constexpr bool same_alpha_bits = BPP==rgba::a;
            microgl::lut::dynamic_lut_bits<BPP, rgba::a, false, same_alpha_bits> _lut_a;

        public:
            BPP_RGB() : _lut_a{} {}

            inline void encode(const color_t &input, uint8_t &output) const {
            }

            inline void decode(const uint8_t &input, color_t &output) const {
                output.r=full_r;
                output.g=full_g;
                output.b=full_b;
                output.a=same_alpha_bits ? input : _lut_a[input];
            };

        };
    }
}