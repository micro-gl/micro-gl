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

#include "../math.h"

namespace microgl {
    namespace functions {

        template<typename number>
        static number length(const number &p_x, const number & p_y) {
            return microgl::math::sqrt(p_x*p_x + p_y*p_y);
        }

        template<typename number>
        static number distance(number p0_x, number p0_y, number p1_x, number p1_y) {
            auto dx= p0_x-p1_x;
            auto dy= p0_y-p1_y;
            return length(dx, dy);
        }
    }
}
