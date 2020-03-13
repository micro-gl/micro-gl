#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Lighten : public BlendModeBase<Lighten> {
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = b.r>s.r ? b.r : s.r;
                output.g = b.g>s.g ? b.g : s.g;
                output.b = b.b>s.b ? b.b : s.b;
            }

            static inline const char *type() {
                return "Lighten";
            }
        };

    }
}