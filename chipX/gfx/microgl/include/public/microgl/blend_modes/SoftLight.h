#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class SoftLight : public BlendModeBase<SoftLight> {
    public:

        static inline float blend_SoftLight(float b, float s) {
            return (s<0.5) ? (2.0*b*s+b*b*(1.0-2.0*s)) : (sqrt(b)*(2.0*s-1.0)+2.0*b*(1.0-s));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_SoftLight(b.r,s.r);
            output.g=blend_SoftLight(b.g,s.g);
            output.b=blend_SoftLight(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

        }

        static inline char* type() {
            return "SoftLight";
        }
    };

}