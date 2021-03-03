#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class LinearBurn : public BlendModeBase<LinearBurn> {
        public:

            template<uint8_t bits>
            static inline
            uint blend_channel(cuint b, cuint s) {
                constexpr cuint max= (uint(1)<<bits)-1;
                cuint res= b+s-max;
                return res<0 ? 0 : res;
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