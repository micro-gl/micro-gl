#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class Screen : public BlendModeBase<Screen> {
    public:
        static inline int blend_Screen(int b, int s, int bits) {
            int max = (1<<bits) - 1;

            return max-(((max-b)*(max-s))>>bits);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_Screen(b.r,s.r,r_bits);
            output.g=blend_Screen(b.g,s.g,g_bits);
            output.b=blend_Screen(b.b,s.b,b_bits);
        }

        static inline float blend_Screen(float b, float s) {
            return 1.0-((1.0-b)*(1.0-s));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_Screen(b.r,s.r);
            output.g=blend_Screen(b.g,s.g);
            output.b=blend_Screen(b.b,s.b);
        }

        static inline const char* type() {
            return "Screen";
        }
    };

}