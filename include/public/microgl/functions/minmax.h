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

        template<typename number>
        inline number max(const number & a, const number & b) {
            return a > b ? a : b;
        }
        template<typename number, typename... Args>
        inline number max(const number & a, const number & b, Args... args) {
            return max<number>(max<number>(a, b), args...);
        }

        template<typename number>
        inline number min(const number & a, const number & b) {
            return a < b ? a : b;
        }
        template<typename number, typename... Args>
        inline number min(const number & a, const number & b, Args... args) {
            return min<number>(min<number>(a, b), args...);
        }

        template<typename number>
        inline number abs_max(const number & a, const number & b) {
            const auto a_abs = a<0 ? -a : a;
            const auto b_abs = b<0 ? -b : b;
            const auto max = a_abs < b_abs ? b_abs : a_abs;
            return max;
        }
        template<typename number, typename... Args>
        inline number abs_max(const number & a, const number & b, Args... args) {
            const auto max = abs_max<number>(a, b);
            return abs_max<number>(max, args...);
        }

        template<typename number>
        inline number abs_min(const number & a, const number & b) {
            const auto a_abs = a<0 ? -a : a;
            const auto b_abs = b<0 ? -b : b;
            const auto min = a_abs > b_abs ? b_abs : a_abs;
            return min;
        }
        template<typename number, typename... Args>
        inline number abs_min(const number & a, const number & b, Args... args) {
            const auto max = abs_min<number>(a, b);
            return abs_min<number>(max, args...);
        }
    }
}
