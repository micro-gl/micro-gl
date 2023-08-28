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

#include <microgl/porter_duff/porter_duff_base.h>

namespace microgl {
    namespace porterduff {

        struct FastSourceOverOnOpaque {

            template <uint8_t bits, bool multiplied_alpha_result=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output) {
                // this is to avoid branching as well
                // here output alpha is always = 1.0, therefore we can optimize,
                // there is no difference between pre to un multiplied result.
                constexpr microgl::ints::uint16_t max_val = microgl::ints::uint16_t(1<<bits)-1;
                const microgl::ints::uint16_t comp = max_val - s.a;
//                unsigned int comp = s.a ^max_val; // flipping bits equals (max_val - s.a)
                output.r = (s.a * s.r + comp * b.r) >> bits;
                output.g = (s.a * s.g + comp * b.g) >> bits;
                output.b = (s.a * s.b + comp * b.b) >> bits;
                output.a = max_val;
            }

        };

    }
}