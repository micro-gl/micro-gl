#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast =true>
        class SoftLight : public BlendModeBase<SoftLight<fast>> {
        public:

            static inline
            uint blend_channel(cuint b, cuint s, const bits bits) {
                cint max= (uint(1)<<bits)-1;
                cint max_double= max*max;

                if(fast)
                    return (((max-2*int(s))*int(b)*int(b))>>(bits<<1)) + (int(2*s*b)>>bits);
                else
                    return (((max-2*int(s))*int(b)*int(b))/max_double) + int(2*s*b)/max;
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
                return "SoftLight";
            }
        };

    }
}