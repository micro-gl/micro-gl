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

//        float sdSegment( in vertex2 p, in vertex2 a, in vertex2 b )
//        {
//            vertex2 pa = p-a, ba = b-a;
//            float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
//            return length( pa - ba*h );
//        }

        /**
         * a capsule sampler
         *
         * @tparam number the number type of inputs
         * @tparam rgba_ rgba info, define alpha bits for transparency
         * @tparam anti_alias apply anti-aliasing
         * @tparam $precision precision
         */
        template <typename number, typename rgba_=rgba_t<8,8,8,8>, bool anti_alias=true,
                enum precision $precision=precision::high>
        class capsule_sampler {
        public:
            using rgba = rgba_;
            using vertex = microgl::vertex2<number>;
        private:
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
        public:
            using rint= typename microgl::traits::conditional<p_bits>=16,
                    microgl::ints::int64_t, microgl::ints::int32_t>::type;
            static constexpr rint ONE= rint(1)<<p_bits;

            using ivertex = vertex2<rint>;

            color_t color_fill= {0, 0, 0, (1u<<rgba::a)-1};
            color_t color_background= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, 0};
            color_t color_stroke= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, (1u<<rgba::a)-1};

            capsule_sampler() = default;

        private:
            rint _fraction_radius, _fraction_stroke;
            rint _reciprocal_a_dot_b;
            ivertex _a, _b;

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

            inline rint sdSegment_squared(const ivertex &p, const ivertex &a,
                                          const ivertex &b, const rint reciprocal_a_dot_b) const
            {
                auto pa = p-a, ba = b-a;
                rint pa_dot_ba = ((pa.x*ba.x)>>p_bits) + ((pa.y*ba.y)>>p_bits);
                const rint div = (pa_dot_ba*reciprocal_a_dot_b)>>(p_bits);
                auto h = div < 0 ? 0 : (div < ONE ? div : ONE);
                auto vv= pa - ivertex((ba.x*h)>>p_bits, (ba.y*h)>>p_bits);
                auto length_squared= ((vv.x*vv.x)>>p_bits) + ((vv.y*vv.y)>>p_bits);
                return (length_squared);
            }

        public:

            void updatePoints(const vertex & a, const vertex & b, number fraction_radius, number fraction_stroke) {
                _fraction_radius = microgl::math::to_fixed((fraction_radius / number(2)) * (fraction_radius / number(2)), p_bits);
                _fraction_stroke = microgl::math::to_fixed((fraction_stroke / number(2)) * (fraction_stroke / number(2)), p_bits);
                _a.x = microgl::math::to_fixed(a.x, p_bits);
                _a.y = microgl::math::to_fixed(a.y, p_bits);
                _b.x = microgl::math::to_fixed(b.x, p_bits);
                _b.y = microgl::math::to_fixed(b.y, p_bits);
                const auto ab = _b - _a;
                rint dot = ((ab.x*ab.x)>>p_bits) +
                                 ((ab.y*ab.y)>>p_bits);
                _reciprocal_a_dot_b = dot==0 ? 0 : (ONE<<p_bits)/dot;
            }

#define aaaa(x) ((x)<0?-(x):(x))

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                ivertex p{u_tag, v_tag};
                rint distance = sdSegment_squared(p, _a, _b,
                               _reciprocal_a_dot_b);
                constexpr rint aa_bits = p_bits - 9 < 0 ? 0 : p_bits - 9;
                constexpr rint aa_bits2 = aa_bits-1;
                constexpr rint aa_band = 1u << aa_bits;
                constexpr rint aa_band2 =1u << aa_bits2;

                distance = (distance) - _fraction_radius;
                rint distance2 = aaaa(distance) - _fraction_stroke;

                output=color_background;
                const bool stroke_flag=color_stroke.a!=0;

                if((distance)<=0) {
                    output=color_fill;
                }
                else if (anti_alias && !stroke_flag && (distance < aa_band)) {
                    const unsigned char factor = ((color_fill.a * (aa_band - distance)) >> aa_bits);
                    output=color_fill;
                    output.a=factor;
                }
                if(!stroke_flag) return;
                if((distance2)<=0) {
                    output=color_stroke;
                }
                else if (anti_alias && (distance2 < aa_band2)) {
//                    output= {255,255,255,255};
//                    output.a=factor;
                        const color_t & st = color_stroke;
                        rint comp = (aa_band2-distance2);
                        output.r = (distance2*output.r + comp*st.r) >> aa_bits2;
                        output.g = (distance2*output.g + comp*st.g) >> aa_bits2;
                        output.b = (distance2*output.b + comp*st.b) >> aa_bits2;
                        output.a = (distance2*output.a + comp*st.a) >> aa_bits2;
                }

            }

        };

    }

}