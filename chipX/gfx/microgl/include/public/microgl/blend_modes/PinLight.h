#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class PinLight : public BlendModeBase<PinLight> {
        public:

            static inline float blend_Darken(float b, float s) {
                return fmin(s, b);
            }

            static inline float blend_Lighten(float b, float s) {
                return fmax(s, b);
            }

            static inline float blend_PinLight(float b, float s) {
                return (s < 0.5) ? blend_Darken(b, (2.0 * s)) : blend_Lighten(b, (2.0 * (s - 0.5)));
            }

            static inline void blend(const color_f_t &b,
                                     const color_f_t &s,
                                     color_f_t &output) {

                output.r = blend_PinLight(b.r, s.r);
                output.g = blend_PinLight(b.g, s.g);
                output.b = blend_PinLight(b.b, s.b);
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
                return "PinLight";
            }
        };

    }
}