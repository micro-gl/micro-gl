#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true>
        class Overlay : public BlendModeBase<Overlay<fast>> {
        public:

            template <bool use_FPU=true>
            static inline int blend_Overlay(int b, int s, int bits) {
                int max = (1<<bits)-1;

                if(fast)
                    return 2 * b < max ? ((2 * b * s) >> bits) : (max - ((2 * (max - b) * (max - s)) >> bits));
                else if(use_FPU)
                    return 2 * b < max ? (float(2 * b * s)/max) : (max - (float(2 * (max - b) * (max - s))/max));
                else
                    return 2 * b < max ? ((2 * b * s)/max) : (max - ((2 * (max - b) * (max - s))/max));
            }

            template <bool use_FPU=true>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = blend_Overlay<use_FPU>(b.r, s.r, r_bits);
                output.g = blend_Overlay<use_FPU>(b.g, s.g, g_bits);
                output.b = blend_Overlay<use_FPU>(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "Overlay";
            }
        };

    }
}