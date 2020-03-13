#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true, bool use_FPU=true>
        class Overlay : public BlendModeBase<Overlay<fast, use_FPU>> {
        public:

            static inline
            uint blend_Overlay(cuint b, cuint s, const bits bits) {
                cuint max = (uint(1)<<bits)-1;
                if(fast)
                    return 2 * b < max ? ((2 * b * s) >> bits) : (max - ((2 * (max - b) * (max - s)) >> bits));
                else if(use_FPU)
                    return 2.0f * float(b) < float(max) ? 2.0f * float(b) * float(s)/float(max) :
                                        (float(max) - ((2.0f * (float(max) - float(b)) * (float(max) - float(s)))/float(max)));
                else
                    return 2 * b < max ? ((2 * b * s)/max) : (max - ((2 * (max - b) * (max - s))/max));
            }

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = blend_Overlay(b.r, s.r, r_bits);
                output.g = blend_Overlay(b.g, s.g, g_bits);
                output.b = blend_Overlay(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "Overlay";
            }
        };

    }
}