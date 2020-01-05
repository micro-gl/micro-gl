#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Subtract : public BlendModeBase<Subtract> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=fmax(b.r-s.r, 0);
            output.g=fmax(b.g-s.g, 0);
            output.b=fmax(b.b-s.b, 0);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=std::max(b.r-s.r, 0);
            output.g=std::max(b.g-s.g, 0);
            output.b=std::max(b.b-s.b, 0);
        }

        static inline const char* type() {
            return "Subtract";
        }
    };

}