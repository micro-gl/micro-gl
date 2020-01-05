#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class ColorBurn : public BlendModeBase<ColorBurn> {
    public:
        static inline int blend_ColorBurn(int b, int s, uint8_t bits) {
            int max = (1<<bits) - 1;

            return (s==0) ? s : std::max((max - ((max-b)<<bits)/s), 0);

//    return (s==0.0)?s:fmax((1.0-((1.0-b)/s)),0.0);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_ColorBurn(b.r, s.r, r_bits);
            output.g=blend_ColorBurn(b.g, s.g, g_bits);
            output.b=blend_ColorBurn(b.b, s.b, b_bits);
        }

        static inline float blend_ColorBurn(float b, float s) {
            return (s==0.0)?s:fmax((1.0-((1.0-b)/s)),0.0);
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_ColorBurn(b.r, s.r);
            output.g=blend_ColorBurn(b.g, s.g);
            output.b=blend_ColorBurn(b.b, s.b);
        }

        static inline const char* type() {
            return "ColorBurn";
        }
    };

}