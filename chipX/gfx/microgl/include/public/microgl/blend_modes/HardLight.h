#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class HardLight : public BlendModeBase<HardLight> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            Overlay::blend(s, b, output);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            Overlay::blend(s, b, output, r_bits, g_bits, b_bits);

        }

        static inline char* type() {
            return "HardLight";
        }
    };

}