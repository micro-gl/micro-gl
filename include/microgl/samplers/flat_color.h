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

#include <microgl/color.h>

namespace microgl {
    namespace sampling {

        template<typename rgba_=rgba_t<8,8,8,0>>
        struct flat_color {
            using rgba = rgba_;
            explicit flat_color()= default;
            explicit flat_color(const color_t & $color) : color{$color} {}

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                // we only copy channel values and not bit information
                output.r=color.r;
                output.g=color.g;
                output.b=color.b;
                output.a=color.a;
//                output=color;
            }
            color_t color;
        };

    }
}