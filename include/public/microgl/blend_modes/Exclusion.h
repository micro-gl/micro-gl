#pragma once

#include <microgl/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true>
        class Exclusion : public blend_mode_base<Exclusion<fast>> {
        private:

            template<uint8_t bits>
            static inline
            uint blend_Exclusion(cuint b, cuint s) {
                constexpr cuint max= (uint(1)<<bits)-1;
                if(fast)
                    return b + s - ((2 * b * s)>>bits);
                else
                    return b + s - 2 * mc<bits>(b, s);
            }

        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                output.r = blend_Exclusion<R>(b.r, s.r);
                output.g = blend_Exclusion<G>(b.g, s.g);
                output.b = blend_Exclusion<B>(b.b, s.b);
            }

        };

    }
}