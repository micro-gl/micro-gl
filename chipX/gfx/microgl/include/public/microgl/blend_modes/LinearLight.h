#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class LinearLight : public BlendModeBase<LinearLight> {
    public:

        static inline float blend_LinearLight(float b, float s) {
            return (s < 0.5) ? LinearBurn::blend_LinearBurn(b,(2.0*s)) : LinearBurn::blend_LinearBurn(b,(2.0*(s-0.5)));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_LinearLight(b.r,s.r);
            output.g=blend_LinearLight(b.g,s.g);
            output.b=blend_LinearLight(b.b,s.b);
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
            return "LinearLight";
        }
    };

}