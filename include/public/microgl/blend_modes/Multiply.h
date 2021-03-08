#pragma once

#include <microgl/blend_mode_base.h>
#include <microgl/channel.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true>
        class Multiply : public blend_mode_base<Multiply<fast>> {
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                if(fast) {
                    using cuint=unsigned int;
                    output.r = (cuint(b.r) * s.r) >> R;
                    output.g = (cuint(b.b) * s.b) >> G;
                    output.b = (cuint(b.b) * s.b) >> B;
                } else {
                    output.r = mul_channels_correct<R>(b.r, s.r);
                    output.g = mul_channels_correct<G>(b.g, s.g);
                    output.b = mul_channels_correct<B>(b.b, s.b);
                }

            }

        };

    }
}