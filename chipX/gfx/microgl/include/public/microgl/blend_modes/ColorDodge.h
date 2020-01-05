#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class ColorDodge : public BlendModeBase<ColorDodge> {
    public:

        static inline int blend_ColorDodge(int b, int s, int bits) {
            int max = (1<<bits) - 1;

            return (s==max) ? s : std::min((b<<bits)/(max-s), max);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_ColorDodge(b.r, s.r, r_bits);
            output.g=blend_ColorDodge(b.g, s.g, g_bits);
            output.b=blend_ColorDodge(b.b, s.b, b_bits);
        }

        static inline float blend_ColorDodge(float b, float s) {
            return (s==1.0)?s:fmin(b/(1.0-s),1.0);
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_ColorDodge(b.r, s.r);
            output.g=blend_ColorDodge(b.g, s.g);
            output.b=blend_ColorDodge(b.b, s.b);
        }

        static inline char* type() {
            return "ColorDodge";
        }
    };

}