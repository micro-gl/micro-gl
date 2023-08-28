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
#include <microgl/stdint.h>

namespace microgl {
    namespace blendmode {

        class ColorBurn {
        private:
            template<microgl::ints::uint8_t bits>
            static inline
            uint blend_ColorBurn(cuint b, cuint s) {
                constexpr cuint max = (uint(1) << bits) - 1;
                if(s==0) return s;
                cuint bb = max - ((max - b)*max) / s;
                return bb<0 ? 0 : bb;
            }

        public:
            template<microgl::ints::uint8_t R, microgl::ints::uint8_t G, microgl::ints::uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                output.r = blend_ColorBurn<R>(b.r, s.r);
                output.g = blend_ColorBurn<G>(b.g, s.g);
                output.b = blend_ColorBurn<B>(b.b, s.b);
            }
        };
    }
}