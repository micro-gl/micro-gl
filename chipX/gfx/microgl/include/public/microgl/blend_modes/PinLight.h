#pragma once

#include <microgl/BlendMode.h>
#include <cmath>

namespace microgl {
    namespace blendmode {

        class PinLight : public BlendModeBase<PinLight> {
        private:
            static inline uint blend_Darken(cuint b, cuint s) {
                return b < s ? b : s;
            }

            static inline uint blend_Lighten(cuint b, cuint s) {
                return b > s ? b : s;
            }

            static inline uint blend_PinLight(cuint b, cuint s, cuint bits) {
                cuint half= (uint(1)<<(bits-1));
                return (s < half) ? blend_Darken(b, (2 * s)) :
                       blend_Lighten(b, (2 * (s - half)));
            }
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = blend_PinLight(b.r, s.r, r_bits);
                output.g = blend_PinLight(b.g, s.g, g_bits);
                output.b = blend_PinLight(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "PinLight";
            }
        };

    }
}