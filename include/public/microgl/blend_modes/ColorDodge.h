#pragma once

#include <microgl/blend_modes/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        class ColorDodge {
        private:
            template<uint8_t bits>
            static inline
            uint blend_ColorDodge(cuint b, cuint s) {
                constexpr cuint max = (uint(1) << bits) - 1;
                if(s==max) return s;
                cuint res= (b * max) / (max - s);
                return res>max ? max : res;
            }

        public:
            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                output.r = blend_ColorDodge<R>(b.r, s.r);
                output.g = blend_ColorDodge<G>(b.g, s.g);
                output.b = blend_ColorDodge<B>(b.b, s.b);
            }
        };
    }
}