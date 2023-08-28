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
#include <microgl/math/vertex2.h>
#include <microgl/math.h>
#include <microgl/samplers/precision.h>
#include <microgl/color.h>

namespace microgl {
    namespace sampling {

        /**
         * a linear classifier sampler. Once two points are updated, a line is formed
         * at the unit rectangle. Then, samples that are left of the line are assigned
         * color_left and everything to the right gets color_right. We also support linear
         * transition that fights aliasing.
         *
         * @tparam number the number type of inputs
         * @tparam rgba_ rgba info, define alpha bits for transparency
         * @tparam anti_alias_bits bits for transition, higher bits produce thicker transition
         * @tparam $precision precision for numbers
         */
        template <typename number, typename rgba_=rgba_t<8,8,8,8>, unsigned anti_alias_bits=7,
                enum precision $precision=precision::high>
        class linear_classifier_sampler {
        public:
            using rgba = rgba_;
            using vertex = microgl::vertex2<number>;
        private:
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
        public:
            using rint= typename microgl::traits::conditional<p_bits>=16,
                    microgl::ints::int64_t, microgl::ints::int32_t>::type;
            using ivertex = microgl::vertex2<rint>;

            color_t color_left= {(1u<<rgba::r)-1, 0, 0, (1u<<rgba::a)-1};
            color_t color_right= {0, (1u<<rgba::g)-1, 0, (1u<<rgba::a)-1};

            linear_classifier_sampler() = default;

        private:
            ivertex _a, _b, _ab;

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

        public:

            void updatePoints(const vertex & a, const vertex & b) {
                _a.x = microgl::math::to_fixed(a.x, p_bits);
                _b.y = microgl::math::to_fixed(a.y, p_bits);
                _b.x = microgl::math::to_fixed(b.x, p_bits);
                _b.y = microgl::math::to_fixed(b.y, p_bits);
                _ab = _b-_a;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                using uint = unsigned int;
                using uc = unsigned char;
                const auto px= convert(u, bits, p_bits);
                const auto py= convert(v, bits, p_bits);
                rint signed_area= -(((px-_a.x)*_ab.y)>>p_bits) + (((py-_a.y)*_ab.x)>>p_bits);
                output=signed_area>=0 ? color_left : color_right;
                if(anti_alias_bits>0) { // compile-time branching
                    constexpr uc aa_bits = anti_alias_bits;
                    constexpr rint aa_band = 1u<<aa_bits;
                    rint dis = aa_band+signed_area;
                    if(dis>0 && (dis<aa_band)) {
                        auto comp = -(signed_area);//signed_area<0?-signed_area:signed_area;
                        output.r=(dis*rint(color_left.r)+comp*rint(color_right.r))>>aa_bits;
                        output.g=(dis*rint(color_left.g)+comp*rint(color_right.g))>>aa_bits;
                        output.b=(dis*rint(color_left.b)+comp*rint(color_right.b))>>aa_bits;
                        output.a=(dis*rint(color_left.a)+comp*rint(color_right.a))>>aa_bits;
                    }
                }

            }

        };

    }

}