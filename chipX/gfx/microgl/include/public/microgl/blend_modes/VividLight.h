#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class VividLight : public BlendModeBase<VividLight> {
        private:
            static inline
            uint blend_ColorBurn(cuint b, cuint s, const bits bits) {
                cuint max = (uint(1) << bits) - 1;
                if(s==0) return s;
                cuint bb = max - ((max - b)*max) / s;
                return bb<0 ? 0 : bb;
            }

            static inline
            uint blend_ColorDodge(cuint b, cuint s, const bits bits) {
                cuint max = (uint(1) << bits) - 1;
                if(s==max) return s;
                cuint bb = (b*max) / (max-s);
                return bb>max ? max : bb;
            }

        public:

            static inline
            uint blend_channel(cuint b, cuint s, const bits bits) {
                cuint half = uint(1)<<(bits-1);
                return (s < half) ? blend_ColorBurn(b, 2 * s, bits) : blend_ColorDodge(b, 2*(s - half), bits);
            }

            static inline
            void blend(const color_t &b,
                       const color_t &s,
                       color_t &output,
                       const uint8_t r_bits,
                       const uint8_t g_bits,
                       const uint8_t b_bits) {
                output.r = blend_channel(b.r, s.r, r_bits);
                output.g = blend_channel(b.g, s.g, g_bits);
                output.b = blend_channel(b.b, s.b, b_bits);
            }

            static inline const char *type() {
                return "VividLight";
            }
        };

    }
}