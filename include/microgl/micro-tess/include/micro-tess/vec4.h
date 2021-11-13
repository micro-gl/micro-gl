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

#include "vec3.h"

namespace microtess {

    template<typename number>
    struct vec4 {

        number x, y, z, w;

        vec4() = default;
        vec4(const number & $x, const number & $y,
             const number & $z, const number & $w=number(0)) :
                x{$x}, y{$y}, z{$z}, w{$w} {}
        vec4(const vec3<number> vec3) : vec4{vec3.x, vec3.y, vec3.z, number{1}} {}

        template<typename F>
        vec4(const vec4<F> & a) {
            this->x = static_cast<number>(a.x);
            this->y = static_cast<number>(a.y);
            this->z = static_cast<number>(a.z);
            this->w = static_cast<number>(a.w);
        }

        vec4 operator-() const { return vec4{-this->x, -this->y, -this->z, -this->w}; }
        vec4 operator+(const vec4 & a) const { return vec4{this->x + a.x, this->y + a.y,
                                                           this->z + a.z, this->w + a.w};
        }
        vec4 operator-(const vec4 & val) const { return *this + (-val); }
        vec4 operator*(const vec4 & a) const { return (this->x*a.x, this->y*a.y, this->z*a.z, this->w*a.w); }
        number dot(const vec4 & a) const { return (this->x*a.x + this->y*a.y+this->z*a.z + this->w*a.w); }
        vec4<number> operator*(const number & val) const {
            return vec4<number>{this->x*val, this->y*val, this->z*val, this->w*val};
        }
        template<typename F>
        vec4<number> operator/(const F & a) const {
            return vec4<number>{this->x/number(a), this->y/number(a),
                           this->z/number(a), this->w/number(a)};
        }

        bool operator==(const vec4 & rhs) const {
            return this->x==rhs.x &&
                   this->y==rhs.y &&
                   this->z==rhs.z &&
                   this->w==rhs.w;
        }

        template<typename F>
        vec4<number> & operator=(const vec4<F> & a) {
            this->x = static_cast<number>(a.x);
            this->y = static_cast<number>(a.y);
            this->z = static_cast<number>(a.z);
            this->w = static_cast<number>(a.w);
            return *this;
        }
    };
}
