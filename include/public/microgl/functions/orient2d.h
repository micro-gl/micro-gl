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

namespace microgl {
    namespace functions {

        template <typename coords, typename result>
        result orient2d(const coords &a_x, const coords &a_y, const coords &b_x, const coords &b_y,
                     const coords &c_x, const coords &c_y, unsigned char precision) {
            return ((result(b_x-a_x)*result(c_y-a_y)) - (result(b_y-a_y)*result(c_x-a_x)))>>precision;
        }

        template<typename number>
        inline number
        orient2d(const number &a_x, const number &a_y, const number &b_x, const number &b_y, const number &c_x,
                 const number &c_y) {
            return (b_x-a_x)*(c_y-a_y) - (b_y-a_y)*(c_x-a_x);
        }
    }
}
