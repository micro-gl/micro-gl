#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Darken : public BlendModeBase<Darken> {
    public:

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=std::min(b.r,s.r);
            output.g=std::min(b.g,s.g);
            output.b=std::min(b.b,s.b);
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=fmin(b.r,s.r);
            output.g=fmin(b.g,s.g);
            output.b=fmin(b.b,s.b);
        }

        static inline char* type() {
            return "Darken";
        }
    };

}