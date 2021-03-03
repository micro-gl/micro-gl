#pragma once

#include <microgl/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        class Difference : public blend_mode_base<Difference> {
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                int r_=b.r - s.r;
                int g_=b.g - s.g;
                int b_=b.b - s.b;
                output.r =r_<0 ? -r_ : r_;
                output.g =g_<0 ? -g_ : g_;
                output.b =b_<0 ? -b_ : b_;
            }

        };

    }
}