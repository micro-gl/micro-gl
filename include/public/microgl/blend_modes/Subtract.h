#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Subtract : public BlendModeBase<Subtract> {
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                int r_= b.r - s.r;
                int g_= b.g - s.g;
                int b_= b.b - s.b;
                output.r = r_ < 0 ? 0 : r_;
                output.g = g_ < 0 ? 0 : g_;
                output.b = b_ < 0 ? 0 : b_;
            }

        };

    }
}