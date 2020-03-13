#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true, bool use_FPU=false>
        class Screen : public BlendModeBase<Screen<fast, use_FPU>> {
        private:
            static inline
            uint blend_Screen(int b, int s, int bits) {
                cuint max = (uint(1)<<bits)-1;
                if(fast)
                    return max - (((max - b) * (max - s)) >> bits);
                else if(use_FPU)
                    return float(max) - (float((max - b) * (max - s)) /float(max));
                else
                    return max - (((max - b) * (max - s)) /max);
            }

        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = blend_Screen(b.r, s.r, r_bits);
                output.g = blend_Screen(b.g, s.g, g_bits);
                output.b = blend_Screen(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "Screen";
            }
        };

    }
}