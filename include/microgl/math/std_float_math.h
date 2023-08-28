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

#include <microgl/math/base_math.h>
#include <cmath>

namespace microgl {
    namespace math {

        inline int to_fixed(const float val, unsigned char precision) {
            return int(val * float(int(1) << precision));
        }
        inline int to_fixed(const double val, unsigned char precision) {
            return int(val * double((long long)(1ull) << precision));
        }
        inline float mod(float numer, float denom) { return fmodf(numer, denom); }
        inline double mod(double numer, double denom) { return fmod(numer, denom); }
        inline float sqrt(const float val) { return std::sqrt(val); }
        inline double sqrt(const double val) { return std::sqrt(val); }
        inline float sin(const float radians) { return std::sin(radians); }
        inline double sin(const double radians) { return std::sin(radians); }
        inline float cos(const float radians) { return std::cos(radians); }
        inline double cos(const double radians) { return std::cos(radians); }
        inline float tan(const float radians) { return std::tan(radians); }
        inline double tan(const double radians) { return std::tan(radians); }
    }
}