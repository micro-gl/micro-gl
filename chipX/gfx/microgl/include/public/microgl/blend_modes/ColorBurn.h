#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class ColorBurn : public BlendModeBase<ColorBurn> {
        private:
            static inline
            uint blend_ColorBurn(cuint b, cuint s, const bits bits) {
                cuint max = (uint(1) << bits) - 1;
                if(s==0) return s;
                cuint bb = max - ((max - b)*max) / s;
                return bb<0 ? 0 : bb;
            }

        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = blend_ColorBurn(b.r, s.r, r_bits);
                output.g = blend_ColorBurn(b.g, s.g, g_bits);
                output.b = blend_ColorBurn(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "ColorBurn";
            }
        };

    }
}