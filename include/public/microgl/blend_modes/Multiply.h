/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include <microgl/blend_modes/blend_mode_base.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true>
        struct Multiply {
            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                if(fast) {
                    using cuint=unsigned int;
                    output.r = (cuint(b.r) * s.r) >> R;
                    output.g = (cuint(b.g) * s.g) >> G;
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