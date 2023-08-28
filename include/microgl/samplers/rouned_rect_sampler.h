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

#include <microgl/color.h>
#include <microgl/math/vertex2.h>
#include <microgl/math.h>
#include <microgl/samplers/precision.h>

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
        class rouned_rect_sampler {
        public:
            using rgba = rgba_;
            using vertex = microgl::vertex2<number>;
        private:
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
        public:
            using rint= typename microgl::traits::conditional<p_bits>=16,
                    microgl::ints::int64_t, microgl::ints::int32_t>::type;
            static constexpr rint ONE= rint(1)<<p_bits;

            using ivertex = microgl::vertex2<rint>;

            color_t color_fill= {0, 0, 0, (1u<<rgba::a)-1};
            color_t color_background= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, 0};
            color_t color_stroke= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, (1u<<rgba::a)-1};

            rouned_rect_sampler() = default;

        private:
            rint _fraction_radius, _fraction_stroke;
            ivertex _center, _dim;

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

        public:
            void updatePoints(const vertex & center, const vertex & dim,
                              number fraction_radius, number fraction_stroke) {
                if(fraction_stroke>fraction_radius) fraction_stroke=fraction_radius;
                _fraction_radius = microgl::math::to_fixed((fraction_radius) * (fraction_radius), p_bits);
                _fraction_stroke = microgl::math::to_fixed((fraction_stroke) * (fraction_stroke), p_bits);
                _center.x = microgl::math::to_fixed(center.x, p_bits);
                _center.y = microgl::math::to_fixed(center.y, p_bits);
                _dim.x = microgl::math::to_fixed(dim.x/2.f - fraction_radius, p_bits);
                _dim.y = microgl::math::to_fixed(dim.y/2.f - fraction_radius, p_bits);
            }

#define aaaa(x) ((x)<0?-(x):(x))
#define mmmm(a,b) ((a>b)?(a):(b))

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                ivertex p{u_tag, v_tag};
                auto pc = p-_center;
                pc.x = aaaa(pc.x) - _dim.x;
                pc.y = aaaa(pc.y) - _dim.y;
                pc.x = pc.x < 0 ? 0 : pc.x;
                pc.y = pc.y < 0 ? 0 : pc.y;
                rint distance= ((pc.x*pc.x)>>p_bits) + ((pc.y*pc.y)>>p_bits);
//                rint distance= mmmm(pc.x, pc.y);

                constexpr rint aa_bits = p_bits - 8 < 0 ? 0 : p_bits - 8;
                constexpr rint aa_bits2 = aa_bits-1;
                constexpr rint aa_band = 1u << aa_bits;
                constexpr rint aa_band2 = 1u << aa_bits2;

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
                    const color_t & st = color_stroke;
                    rint comp = (aa_band2-distance2);
                    output.r = (output.r*distance2 + st.r*(comp)) >> aa_bits2;
                    output.g = (output.g*distance2 + st.g*(comp)) >> aa_bits2;
                    output.b = (output.b*distance2 + st.b*(comp)) >> aa_bits2;
                    output.a = (output.a*distance2 + st.a*(comp)) >> aa_bits2;
                }

            }
#undef aaaa
        };

    }

}