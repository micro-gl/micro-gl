#pragma once

#include <microgl/blend_modes/LinearBurn.h>

namespace microgl {
    namespace blendmode {

        class LinearLight : public BlendModeBase<LinearLight> {
        public:

            static inline
            uint blend_channel(cuint b, cuint s, const bits bits) {
                cuint max= (uint(1)<<bits)-1;
                cuint res= LinearBurn::blend_channel(b, 2*s, bits);
                return res>max ? max : res;
            }

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = blend_channel(b.r, s.r, r_bits);
                output.g = blend_channel(b.g, s.g, g_bits);
                output.b = blend_channel(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "LinearLight";
            }
        };

    }
}