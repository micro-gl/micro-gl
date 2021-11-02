#pragma once

#include <microgl/blend_modes/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        class HardMix {
        private:
            template<uint8_t bits>
            static inline uint
            blend_HardMix(cuint b, cuint s) {
                constexpr cuint half= uint(1)<<(bits-1);
                constexpr cuint max= (uint(1)<<bits)-1;
                return (VividLight::blend_channel<bits>(b, s) < half) ? 0 : max;
            }

        public:
            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                output.r = blend_HardMix<R>(b.r, s.r);
                output.g = blend_HardMix<G>(b.g, s.g);
                output.b = blend_HardMix<B>(b.b, s.b);
            }
        };
    }
}