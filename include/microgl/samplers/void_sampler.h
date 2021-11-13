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

namespace microgl {
    namespace sampling {

        /**
         * void sampler is a distinguished sampler in micro-gl, this sampler
         * has 0 bits for each channel and the rasterizers are aware of it and
         * do not draw.
         */
        struct void_sampler {
            using rgba = rgba_t<0,0,0,0>;
            explicit void_sampler()= default;

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
            }
        };
    }
}