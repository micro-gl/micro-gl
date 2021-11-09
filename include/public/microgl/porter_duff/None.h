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

        template <bool fast=true>
        struct None {

            template <uint8_t bits, bool multiplied_alpha_result=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output) {
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
                output.a = s.a;
                if (multiplied_alpha_result) {
                    if(fast) {
                        output.r = (uint(output.r) * output.a) >> bits;
                        output.g = (uint(output.g) * output.a) >> bits;
                        output.b = (uint(output.b) * output.a) >> bits;
                    } else {
                        output.r = mc<bits>(output.r, output.a);
                        output.g = mc<bits>(output.g, output.a);
                        output.b = mc<bits>(output.b, output.a);
                    }

                }

            }

        };

    }
}