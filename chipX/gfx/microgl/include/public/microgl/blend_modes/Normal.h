#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Normal : public BlendModeBase<Normal> {
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
            }
            static inline const char *type() {
                return "Normal";
            }

        };

    }
}