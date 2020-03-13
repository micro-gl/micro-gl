#pragma once

#include <microgl/blend_modes/Overlay.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true, bool use_FPU=true>
        class HardLight : public BlendModeBase<HardLight<fast, use_FPU>> {
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                Overlay<fast, use_FPU>::blend(s, b, output, r_bits, g_bits, b_bits);
            }

            static inline const char *type() {
                return "HardLight";
            }
        };

    }
}