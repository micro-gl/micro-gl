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

        class VividLight {
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
            template<uint8_t bits>
            static inline
            uint blend_channel(cuint b, cuint s) {
                constexpr cuint half = uint(1)<<(bits-1);
                return (s < half) ? blend_ColorBurn(b, 2 * s, bits) : blend_ColorDodge(b, 2*(s - half), bits);
            }

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline
            void blend(const color_t &b,
                       const color_t &s,
                       color_t &output) {
                output.r = blend_channel<R>(b.r, s.r);
                output.g = blend_channel<G>(b.g, s.g);
                output.b = blend_channel<B>(b.b, s.b);
            }
        };
    }
}