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
        template <unsigned BPP, typename rgba_>
        class BPP_RGBA {
        public:
            using u8 = unsigned char;
            using rgba = rgba_;
            using pixel = u8;
        private:
            // dynamic look-up tables
            static constexpr u8 full_r = (1u<<rgba::r) - 1;
            static constexpr u8 full_g = (1u<<rgba::g) - 1;
            static constexpr u8 full_b = (1u<<rgba::b) - 1;
            static constexpr bool same_alpha_bits = BPP==rgba::a;
            microgl::lut::dynamic_lut_bits<BPP, rgba::a, false, same_alpha_bits> _lut_a;

        public:
            BPP_RGB() : _lut_a{} {}

            inline void encode(const color_t &input, u8 &output) const {
            }

            inline void decode(const u8 &input, color_t &output) const {
                output.r=full_r;
                output.g=full_g;
                output.b=full_b;
                output.a=same_alpha_bits ? input : _lut_a[input];
            };

        };
    }
}