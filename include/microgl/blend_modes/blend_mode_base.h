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

#include "../color.h"
#include "../stdint.h"
#include "../traits.h"

namespace microgl {
    namespace blendmode {
        using bits = unsigned char;
        using uint = unsigned int;
        using cuint = const unsigned int;
        using cint = const int;
        using uint8_t = microgl::ints::uint8_t;

        template<typename impl>
        class blend_mode_base : public microgl::traits::crpt<impl> {
        protected:
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                impl::template blend<R, G, B>(b, s, output);
            }
        };
    }
}