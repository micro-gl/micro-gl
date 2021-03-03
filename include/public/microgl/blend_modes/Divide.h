#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Divide : public BlendModeBase<Divide> {
        private:

            template<uint8_t bits>
            static inline
            uint blend_Divide(cuint b, cuint s) {
                constexpr cuint max = (uint(1)<<bits)-1;
                cuint res= (s==0) ? max : ((b*max) / s);
                return res>max ? max: res;
            }

        public:
            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                output.r = blend_Divide<R>(b.r, s.r);
                output.g = blend_Divide<G>(b.g, s.g);
                output.b = blend_Divide<B>(b.b, s.b);
            }

        };

    }
}