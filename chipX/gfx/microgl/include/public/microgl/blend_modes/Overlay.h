#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Overlay : public BlendModeBase<Overlay> {
    public:

        static inline int blend_Overlay(int b, int s, int bits) {
            int max = MAX_VAL_BITS(bits);

            return 2*b<max ? ((2*b*s)>>bits) : (max - ((2*(max-b)*(max-s))>>bits));
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_Overlay(b.r,s.r, r_bits);
            output.g=blend_Overlay(b.g,s.g, g_bits);
            output.b=blend_Overlay(b.b,s.b, b_bits);
        }

        static inline float blend_Overlay(float b, float s) {
            return b<0.5?(2.0*b*s):(1.0-2.0*(1.0-b)*(1.0-s));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_Overlay(b.r, s.r);
            output.g=blend_Overlay(b.g, s.g);
            output.b=blend_Overlay(b.b ,s.b);
        }

        static inline char* type() {
            return "Overlay";
        }
    };

}