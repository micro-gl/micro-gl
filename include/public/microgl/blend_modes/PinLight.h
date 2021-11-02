#pragma once

#include <microgl/blend_modes/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        class PinLight {
        private:
            static inline uint blend_Darken(cuint b, cuint s) {
                return b < s ? b : s;
            }

            static inline uint blend_Lighten(cuint b, cuint s) {
                return b > s ? b : s;
            }

        public:

            template<uint8_t bits>
            static inline uint blend_channel(cuint b, cuint s) {
                constexpr cuint half= (uint(1)<<(bits-1));
                return (s < half) ? blend_Darken(b, (2 * s)) :
                       blend_Lighten(b, (2 * (s - half)));
            }

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                output.r = blend_channel<R>(b.r, s.r);
                output.g = blend_channel<G>(b.g, s.g);
                output.b = blend_channel<B>(b.b, s.b);
            }
        };
    }
}