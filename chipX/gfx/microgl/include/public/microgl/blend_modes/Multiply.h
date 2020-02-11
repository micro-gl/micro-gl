#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Multiply : public BlendModeBase<Multiply> {
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = (b.r * s.r) >> r_bits;
                output.g = (b.g * s.g) >> g_bits;
                output.b = (b.b * s.b) >> b_bits;

            }

            static inline void blend(const color_f_t &b,
                                     const color_f_t &s,
                                     color_f_t &output) {

                output.r = (b.r * s.r);
                output.g = (b.g * s.g);
                output.b = (b.b * s.b);
            }

            static inline const char *type() {
                return "Multiply";
            }

        };

    }
}