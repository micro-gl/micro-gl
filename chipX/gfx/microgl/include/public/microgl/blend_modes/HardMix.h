#pragma once

#include <microgl/blend_modes/VividLight.h>

namespace microgl {
    namespace blendmode {

        class HardMix : public BlendModeBase<HardMix> {
        private:
            static inline uint
            blend_HardMix(cuint b, cuint s, const bits bits) {
                cuint half= uint(1)<<(bits-1);
                cuint max= (uint(1)<<bits)-1;
                return (VividLight::blend_channel(b, s, bits) < half) ? 0 : max;
            }

        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                output.r = blend_HardMix(b.r, s.r, r_bits);
                output.g = blend_HardMix(b.g, s.g, g_bits);
                output.b = blend_HardMix(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "HardMix";
            }
        };

    }
}