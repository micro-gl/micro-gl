#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast =true>
        class SoftLight : public BlendModeBase<SoftLight<fast>> {
        public:

            template<uint8_t bits>
            static inline
            uint blend_channel(cuint b, cuint s) {
                constexpr cint max= (uint(1)<<bits)-1;
                constexpr cint max_double= max*max;

                if(fast)
                    return (((max-2*int(s))*int(b)*int(b))>>(bits<<1)) + (int(2*s*b)>>bits);
                else
                    return (((max-2*int(s))*int(b)*int(b))/max_double) + int(2*s*b)/max;
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