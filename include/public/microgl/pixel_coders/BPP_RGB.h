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

namespace microgl {
    namespace coder {

        /**
         * map single channel with BPP bits to R, G, B
         * Alpha channel is muted, i.e 0 bits.
         *
         * @tparam BPP bits of channel
         * @tparam R red channel bits
         * @tparam G green channel bits
         * @tparam B blue channel bits
         */
        template <unsigned BPP, unsigned R, unsigned G, unsigned B>
        class BPP_RGB {
        public:
            using u8 = unsigned char;
            using rgba = rgba_t<R, G, B, 0>;
            using pixel = u8;
        private:
            constexpr static bool same_r = BPP==rgba::r;
            constexpr static bool same_g = BPP==rgba::g;
            constexpr static bool same_b = BPP==rgba::b;

            // dynamic look-up tables
            microgl::lut::dynamic_lut_bits<BPP, R, false, same_r> _lut_r;
            microgl::lut::dynamic_lut_bits<BPP, G, false, same_g> _lut_g;
            microgl::lut::dynamic_lut_bits<BPP, B, false, same_b> _lut_b;

        public:
            BPP_RGB() : _lut_r{}, _lut_g{}, _lut_b{} {}

            inline void encode(const color_t &input, u8 &output) const {
            }

            inline void decode(const u8 &input, color_t &output) const {
                output.r=same_r ? input : _lut_r[input];
                output.g=same_g ? input : _lut_g[input];
                output.b=same_b ? input : _lut_b[input];
            };

        };
    }
}