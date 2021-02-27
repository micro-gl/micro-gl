#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class ColorDodge : public BlendModeBase<ColorDodge> {
        private:
            static inline
            uint blend_ColorDodge(cuint b, cuint s, const bits bits) {
                cuint max = (uint(1) << bits) - 1;
                if(s==max) return s;
                cuint res= (b * max) / (max - s);
                return res>max ? max : res;
            }

        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = blend_ColorDodge(b.r, s.r, r_bits);
                output.g = blend_ColorDodge(b.g, s.g, g_bits);
                output.b = blend_ColorDodge(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "ColorDodge";
            }
        };

    }
}