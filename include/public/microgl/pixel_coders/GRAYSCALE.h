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

#include "RGBA_PACKED.h"

namespace microgl {
    namespace coder {

        template<unsigned bits, unsigned alpha_bits = 0>
        struct GRAYSCALE {
        private:
            constexpr static uint8_t mask = (1u<<bits) - 1;
        public:
            using rgba = rgba_t<bits, bits, bits, alpha_bits>;
            using pixel = unsigned char;

            inline void encode(const color_t &input, pixel &output) const {
                output = input.r & mask;
            }

            inline void decode(const pixel &input, color_t &output) const {
                output.r = output.g = output.b = input;
                if(alpha_bits!=0) output.a = input;
            };

        };

    }
}