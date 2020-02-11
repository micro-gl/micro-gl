#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class LinearDodge : public BlendModeBase<LinearDodge> {
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = std::min(b.r + s.r, (1 << r_bits) - 1);
                output.g = std::min(b.g + s.g, (1 << g_bits) - 1);
                output.b = std::min(b.b + s.b, (1 << b_bits) - 1);
            }

            static inline void blend(const color_f_t &b,
                                     const color_f_t &s,
                                     color_f_t &output) {

                output.r = fmin(b.r + s.r, 1.0f);
                output.g = fmin(b.g + s.g, 1.0f);
                output.b = fmin(b.b + s.b, 1.0f);
            }

            static inline const char *type() {
                return "LinearDodge";
            }
        };

    }
}