#pragma once

#include <microgl/math.h>

namespace microgl {

    template<typename number>
    struct vec3 {

        number x, y, z;

        vec3() = default;
        vec3(const number & $x, const number & $y, const number & $z=number(1)) :
                x{$x}, y{$y}, z{$z} {}

        template<typename F>
        vec3(const vec3<F> & a) {
            this->x = static_cast<number>(a.x);
            this->y = static_cast<number>(a.y);
            this->z = static_cast<number>(a.z);
        }

        vec3 operator-() const {
            return vec3{-this->x, -this->y,
                        -this->z};
        }

        vec3 operator+(const vec3 & a) const {
            return vec3{this->x + a.x, this->y + a.y, this->z + a.z};
        }

        vec3 operator-(const vec3 & a) const {
            return *this + (-a);
        }

        vec3 operator*(const number & val) const {
            return vec3{this->x*val, this->y*val, this->z*val};
        }

        vec3 operator*(const vec3 & a) const {
            return (this->x*a.x, this->y*a.y, this->z*a.z);
        }

        number dot(const vec3 & a) const {
            return (this->x*a.x + this->y*a.y+this->z*a.z);
        }

        vec3 operator/(const number & val) const {
            return {this->x/val, this->y/val, this->z/val};
        }

        bool operator==(const vec3 & rhs) const {
            return this->x==rhs.x && this->y==rhs.y && this->z==rhs.z;
        }

        vec3 & operator=(const vec3 & a) {
            this->x = a.x;this->y = a.y;this->z = a.z;
            return *this;
        }

        vec3 & normalize() {
            auto d = (x*x) + (y*y) + (z*z);
            if(d==number(0))
                return *this;
            auto inv_len = number(1) / microgl::math::sqrt(d);
            x *= inv_len;
            y *= inv_len;
            z *= inv_len;
            return *this;
        }

        vec3 normalize() const {
            vec3 temp{*this};
            temp.normalize();
            return temp;
        }

        vec3 cross(const vec3& rhs) const {
            return {y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x};
        }

    };

}
