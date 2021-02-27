#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Divide : public BlendModeBase<Divide> {
        private:
            static inline
            uint blend_Divide(cuint b, cuint s, const bits bits) {
                cuint max = (uint(1)<<bits)-1;
                cuint res= (s==0) ? max : ((b*max) / s);
                return res>max ? max: res;
            }

        public:
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = blend_Divide(b.r, s.r, r_bits);
                output.g = blend_Divide(b.g, s.g, g_bits);
                output.b = blend_Divide(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "Divide";
            }
        };

    }
}