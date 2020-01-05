#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Exclusion : public BlendModeBase<Exclusion> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r = b.r + s.r - 2.0*b.r*s.r;
            output.g = b.g + s.g - 2.0*b.g*s.g;
            output.b = b.b + s.b - 2.0*b.b*s.b;
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline char* type() {
            return "Exclusion";
        }
    };

}