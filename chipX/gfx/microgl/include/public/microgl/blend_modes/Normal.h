#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Normal : public BlendModeBase<Normal> {
    public:

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r = s.r;
            output.g = s.g;
            output.b = s.b;

        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r = s.r;
            output.g = s.g;
            output.b = s.b;

        }

        static inline char* type() {
            return "Normal";
        }

    };

}