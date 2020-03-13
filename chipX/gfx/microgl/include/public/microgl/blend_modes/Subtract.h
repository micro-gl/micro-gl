#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Subtract : public BlendModeBase<Subtract> {
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                int r_= b.r - s.r;
                int g_= b.g - s.g;
                int b_= b.b - s.b;
                output.r = r_ < 0 ? 0 : r_;
                output.g = g_ < 0 ? 0 : g_;
                output.b = b_ < 0 ? 0 : b_;
            }

            static inline const char *type() {
                return "Subtract";
            }
        };

    }
}