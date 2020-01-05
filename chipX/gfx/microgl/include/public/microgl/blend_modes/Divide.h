#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Divide : public BlendModeBase<Divide> {
    public:
        static inline float blend_Divide(float b, float s) {
            return (s==0.0) ? 1.0 : (b/s);
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_Divide(b.r,s.r);
            output.g=blend_Divide(b.g,s.g);
            output.b=blend_Divide(b.b,s.b);
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
            return "Divide";
        }
    };

}