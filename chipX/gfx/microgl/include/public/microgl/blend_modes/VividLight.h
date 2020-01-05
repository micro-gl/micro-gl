#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class VividLight : public BlendModeBase<VividLight> {
    public:

        static inline float blend_ColorDodge(float b, float s) {
            return (s==1.0)?s:fmin(b/(1.0-s),1.0);
        }

        static inline float blend_VividLight(float b, float s) {
            return (s < 0.5) ? ColorBurn::blend_ColorBurn(b, (2.0*s)) : blend_ColorDodge(b,(2.0*(s-0.5)));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_VividLight(b.r,s.r);
            output.g=blend_VividLight(b.g,s.g);
            output.b=blend_VividLight(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline const char* type() {
            return "VividLight";
        }
    };

}