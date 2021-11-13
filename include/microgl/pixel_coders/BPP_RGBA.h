/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "lut_bits.h"
#include "../color.h"

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
            BPP_RGBA() : _lut_a{} {}

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