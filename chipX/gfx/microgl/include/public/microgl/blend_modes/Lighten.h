#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Lighten : public BlendModeBase<Lighten> {
        public:

            static inline void blend(const color_f_t &b,
                                     const color_f_t &s,
                                     color_f_t &output) {

                output.r = fmax(b.r, s.r);
                output.g = fmax(b.g, s.g);
                output.b = fmax(b.b, s.b);
            }

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                output.r = std::max(b.r, s.r);
                output.g = std::max(b.g, s.g);
                output.b = std::max(b.b, s.b);
            }

            static inline const char *type() {
                return "Lighten";
            }
        };

    }
}