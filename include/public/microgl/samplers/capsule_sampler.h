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
         * @tparam $precision precision
         */
        template <typename number, typename rgba_=rgba_t<8,8,8,8>,
                enum precision $precision=precision::high>
        class capsule_sampler {
        public:
            using rgba = rgba_;
            using vertex = vec2<number>;
        private:
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
            static constexpr bool useBigIntegers = p_bits>=16;
        public:
            using rint= typename microgl::traits::conditional<useBigIntegers,
                                        int64_t, int32_t>::type;
            static constexpr rint ONE= rint(1)<<p_bits;

            using ivertex = vec2<rint>;

            color_t color1= {0,0,0, 255};
            color_t color2= {255,0,0, 0};

            capsule_sampler() = default;

        private:
            rint _epsilon;
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

            void updatePoints(const vertex & a, const vertex & b, number epsilon) {
                _epsilon = microgl::math::to_fixed((epsilon/number(2))*(epsilon/number(2)), p_bits);
                _a.x = microgl::math::to_fixed(a.x, p_bits);
                _a.y = microgl::math::to_fixed(a.y, p_bits);
                _b.x = microgl::math::to_fixed(b.x, p_bits);
                _b.y = microgl::math::to_fixed(b.y, p_bits);
                const auto ab = _b - _a;
                const rint dot = ((ab.x*ab.x)>>p_bits) +
                                 ((ab.y*ab.y)>>p_bits);
                _reciprocal_a_dot_b = (ONE<<p_bits) / dot;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                ivertex p{u_tag, v_tag};
                rint distance = sdSegment_squared(p, _a, _b,
                               _reciprocal_a_dot_b);
                constexpr rint aa_bits = p_bits - 8 < 0 ? 0 : p_bits - 8;
                constexpr rint aa_band = 1u << aa_bits;

                distance -= _epsilon;

                output=color2;

                if((distance)<=0) {
                    output=color1;
                }
                else if (distance < aa_band) {
                    const unsigned char factor = ((color1.a*(aa_band-distance)) >> aa_bits);
                    output=color1;
                    output.a=factor;
                }

            }

        };

    }

}