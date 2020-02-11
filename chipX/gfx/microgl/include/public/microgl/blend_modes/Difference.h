#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Difference : public BlendModeBase<Difference> {
        public:

            static inline void blend(const color_f_t &b,
                                     const color_f_t &s,
                                     color_f_t &output) {

                output.r = fabs(b.r - s.r);
                output.g = fabs(b.g - s.g);
                output.b = fabs(b.b - s.b);
            }

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = abs(b.r - s.r);
                output.g = abs(b.g - s.g);
                output.b = abs(b.b - s.b);
            }

            static inline const char *type() {
                return "Difference";
            }
        };

    }
}