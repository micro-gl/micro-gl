#pragma once

#include <microgl/BlendMode.h>

namespace blendmode {

    class LinearBurn : public BlendModeBase<LinearBurn> {
    public:

        static inline int blend_LinearBurn(int b, int s, uint8_t bits) {
            return std::max(b + s - (1<<bits) - 1, 0);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r = blend_LinearBurn(b.r, s.r, r_bits);
            output.g = blend_LinearBurn(b.g, s.g, g_bits);
            output.b = blend_LinearBurn(b.b, s.b, b_bits);
        }

        static inline float blend_LinearBurn(float b, float s) {
            return fmax(b + s - 1.0, 0.0f);
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r = blend_LinearBurn(b.r, s.r);
            output.g = blend_LinearBurn(b.g, s.g);
            output.b = blend_LinearBurn(b.b, s.b);
        }

        static inline char* type() {
            return "LinearBurn";
        }
    };

}