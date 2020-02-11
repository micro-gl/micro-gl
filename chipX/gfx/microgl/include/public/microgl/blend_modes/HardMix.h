#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class HardMix : public BlendModeBase<HardMix> {
        public:

            static inline float blend_HardMix(float b, float s) {
                return (VividLight::blend_VividLight(b, s) < 0.5) ? 0.0 : 1.0;
            }

            static inline void blend(const color_f_t &b,
                                     const color_f_t &s,
                                     color_f_t &output) {

                output.r = blend_HardMix(b.r, s.r);
                output.g = blend_HardMix(b.g, s.g);
                output.b = blend_HardMix(b.b, s.b);
            }

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {
                // todo
            }

            static inline const char *type() {
                return "HardMix";
            }
        };

    }
}