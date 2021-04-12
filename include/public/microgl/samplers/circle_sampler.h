#pragma once

#include <microgl/rgba_t.h>
#include <microgl/vec2.h>
#include <microgl/math.h>
#include <microgl/precision.h>

namespace microgl {
    namespace sampling {

//        float sdSegment( in vec2 p, in vec2 a, in vec2 b )
//        {
//            vec2 pa = p-a, ba = b-a;
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
        class circle_sampler {
        public:
            using rgba = rgba_;
            using vertex = vec2<number>;
        private:
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
        public:
            using rint= typename microgl::traits::conditional<p_bits>=16,
                                        int64_t, int32_t>::type;
            static constexpr rint ONE= rint(1)<<p_bits;

            using ivertex = vec2<rint>;

            color_t color_fill= {0, 0, 0, (1u<<rgba::a)-1};
            color_t color_background= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, 0};
            color_t color_stroke= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, (1u<<rgba::a)-1};

            circle_sampler() = default;

        private:
            rint _fraction_radius, _fraction_stroke;
            ivertex _center;

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

        public:

            void updatePoints(const vertex & center, number fraction_radius, number fraction_stroke) {
                _fraction_radius = microgl::math::to_fixed((fraction_radius / number(2)) * (fraction_radius / number(2)), p_bits);
                _fraction_stroke = microgl::math::to_fixed((fraction_stroke / number(2)) * (fraction_stroke / number(2)), p_bits);
                _center.x = microgl::math::to_fixed(center.x, p_bits);
                _center.y = microgl::math::to_fixed(center.y, p_bits);
            }

#define aaaa(x) (x)<0?-(x):(x)

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                ivertex p{u_tag, v_tag};
                auto pc = p-_center;
                rint distance= ((pc.x*pc.x)>>p_bits) + ((pc.y*pc.y)>>p_bits);

                constexpr rint aa_bits = p_bits - 9 < 0 ? 0 : p_bits - 9;
                constexpr rint aa_bits2 = aa_bits-1;
                constexpr rint aa_band = 1u << aa_bits;
                constexpr rint aa_band2 =1u << aa_bits2;

                distance = (distance) - _fraction_radius;
                rint distance2 = aaaa(distance) - _fraction_stroke;

                output=color_background;

                if((distance)<=0) {
                    output=color_fill;
                }
                else if (anti_alias && (distance < aa_band)) {
                    const unsigned char factor = ((color_fill.a * (aa_band - distance)) >> aa_bits);
                    output=color_fill;
                    output.a=factor;
                }

                if((distance2)<=0) {
                    output=color_stroke;
                }
                else if (anti_alias && (distance2 < aa_band2)) {
                    const unsigned char factor = ((output.a*(aa_band-distance2)) >> aa_bits);
                        const color_t & st = color_stroke;
                        output.r = (output.r*distance2 + st.r*(aa_band2-distance2)) >> aa_bits2;
                        output.g = (output.g*distance2 + st.g*(aa_band2-distance2)) >> aa_bits2;
                        output.b = (output.b*distance2 + st.b*(aa_band2-distance2)) >> aa_bits2;
                        output.a = (output.a*distance2 + st.a*(aa_band2-distance2)) >> aa_bits2;
                }

            }

        };

    }

}