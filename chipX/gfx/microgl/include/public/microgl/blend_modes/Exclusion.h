#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true>
        class Exclusion : public BlendModeBase<Exclusion<fast>> {
        private:
            static inline
            uint blend_Exclusion(cuint b, cuint s, const bits bits) {
                cuint max= (uint(1)<<bits)-1;
                if(fast)
                    return b + s - ((2 * b * s)>>bits);
                else
                    return b + s - (2 * b * s)/max;
            }

        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = blend_Exclusion(b.r, s.r, r_bits);
                output.g = blend_Exclusion(b.g, s.g, g_bits);
                output.b = blend_Exclusion(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "Exclusion";
            }
        };

    }
}