#pragma once

#include <microgl/blend_modes/LinearBurn.h>

namespace microgl {
    namespace blendmode {

        class LinearLight : public blend_mode_base<LinearLight> {
        public:

            template<uint8_t bits>
            static inline
            uint blend_channel(cuint b, cuint s) {
                constexpr cuint max= (uint(1)<<bits)-1;
                cuint res= LinearBurn::blend_channel<bits>(b, 2*s);
                return res>max ? max : res;
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