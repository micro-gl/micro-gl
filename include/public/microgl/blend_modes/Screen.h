#pragma once

#include <microgl/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true, bool use_FPU=false>
        class Screen : public blend_mode_base<Screen<fast, use_FPU>> {

        public:

            template<uint8_t bits>
            static inline
            uint blend_channel(int b, int s) {
                constexpr cuint max = (uint(1)<<bits)-1;
                if(fast)
                    return max - (((max - b) * (max - s)) >> bits);
                else if(use_FPU)
                    return float(max) - (float((max - b) * (max - s)) /float(max));
                else
                    return max - mc<bits>(max-b, max-s);
            }

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                output.r = blend_channel<R>(b.r, s.r);
                output.g = blend_channel<G>(b.g, s.g);
                output.b = blend_channel<B>(b.b, s.b);
            }

        };

    }
}