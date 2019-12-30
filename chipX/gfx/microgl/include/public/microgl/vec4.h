#pragma once

namespace microgl {

    template<typename T>
    struct vec4 {

        T x, y, z, w;

        vec4() {

        }

        template<typename F>
        vec4(const vec4<F> & a) {
            this->x = static_cast<T>(a.x);
            this->y = static_cast<T>(a.y);
            this->z = static_cast<T>(a.z);
            this->w = static_cast<T>(a.w);
        }

        vec4 operator-() const {
            return vec4{-this->x, -this->y,
                        -this->z, -this->w};
        }

        vec4 operator+(const vec4 & a) const {
            return vec4{this->x + a.x, this->y + a.y,
                        this->z + a.z, this->w + a.w};
        }
        vec4 operator-(const vec4 & a) const {
            return *this + (-a);
        }


        T operator*(const vec4 & a) {
            return (this->x*a.x + this->y*a.y+this->z*a.z + this->w*a.w);
        }


        template<typename F>
        vec4<T> operator*(const F & a) const {
            return vec4<T>{this->x*T(a), this->y*T(a),
                           this->z*T(a), this->z*T(a)};
        }

        template<typename F>
        vec4<T> operator/(const F & a) const {
            return vec4<T>{this->x/T(a), this->y/T(a),
                           this->z/T(a), this->z/T(a)};
        }

        bool operator==(const vec4 & rhs) const {
            return this->x==rhs.x &&
                   this->y==rhs.y &&
                   this->z==rhs.z &&
                   this->w==rhs.w;
        }

        template<typename F>
        vec4<T> & operator=(const vec4<F> & a) {
            this->x = static_cast<T>(a.x);
            this->y = static_cast<T>(a.y);
            this->z = static_cast<T>(a.z);
            this->w = static_cast<T>(a.w);
            return *this;
        }

    };

}
