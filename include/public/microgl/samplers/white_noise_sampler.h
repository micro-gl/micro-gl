#pragma once

#include <microgl/rgba.h>
#include <microgl/math.h>
#include <microgl/precision.h>
#include <microgl/Q.h>

namespace microgl {
    namespace sampling {

        /**
         * a crude white noise sampler
         *
         * @tparam block_bits the block bits
         * @tparam rgba_ rgba info, define alpha bits for transparency
         */
        template <unsigned block_bits, typename rgba_=rgba_t<8,8,8,8>>
        class white_noise_sampler {
        public:
            using rgba = rgba_;

            using rint= typename microgl::traits::conditional<block_bits>=16,
                    int64_t, int32_t>::type;
            using q = Q<14, int64_t>;

            white_noise_sampler() = default;

        private:
            unsigned char max_alpha = (1u<<rgba::a)-1;
            unsigned char max_red = (1u<<rgba::r)-1;
            mutable int ix = 1;

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

        public:

            void update() {
                ix++;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, block_bits);
                const auto v_tag= convert(v, bits, block_bits);
                q q_u{u_tag, block_bits};
                q q_v{v_tag, block_bits};
                auto frac = (microgl::math::sin(q_u*(ix) + q_v*(ix<<10))*43758).fraction();
                unsigned char color = (frac*max_red)>>q::precision;
                output = {color, color, color, max_alpha};
            }

        };

    }

}