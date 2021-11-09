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

namespace microtess {

    template<typename number>
    struct vec2 {
        number x, y;
        vec2() = default;
        constexpr vec2(const number & x_, const number & y_) :
                x{x_}, y{y_} {}

        template<typename F>
        explicit vec2(const vec2<F> & a) {
            this->x = static_cast<number>(a.x);
            this->y = static_cast<number>(a.y);
        }

        vec2 operator-(const vec2 & a) const { return vec2{this->x-a.x, this->y - a.y}; }
        vec2 operator+(const vec2 & a) const { return vec2{this->x + a.x, this->y + a.y}; }
        vec2 operator*(const vec2 & a) const { return {this->x*a.x, this->y*a.y}; }
        number dot(const vec2 & a) const { return (this->x*a.x + this->y*a.y); }
        vec2 orthogonalLeft() const { return {this->y, -this->x}; }
        vec2 orthogonalRight() const { return {-this->y, this->x}; }
        vec2 square() const { return {this->x*this->x, this->y*this->y}; }
        number sum() const { return this->x+this->y; }
        vec2 operator*(const number & val) const { return {this->x*val, this->y*val}; }
        vec2 operator-() const { return vec2<number>{-this->x, -this->y}; }
        vec2 operator/(const number & val) const { return vec2<number>{this->x/val, this->y/val}; }
        vec2 operator/(const vec2 & val) const { return vec2<number>{this->x/val.x, this->y/val.y}; }
        bool operator==(const vec2 & rhs) const { return this->x==rhs.x && this->y==rhs.y; }
        bool operator!=(const vec2 & rhs) const { return !(*this==rhs); }
        vec2 & operator=(const vec2 & a) { this->x = a.x; this->y = a.y; return *this; }
    };
}
