#pragma once
#include <cstdint>

namespace microgl {

    template<typename number>
    struct vec2 {

        number x, y;

        vec2() = default;
        vec2(const number & x_, const number & y_) {
            x = x_;
            y = y_;
        }

        template<typename F>
        explicit vec2(const vec2<F> & a) {
            this->x = static_cast<number>(a.x);
            this->y = static_cast<number>(a.y);
        }

        vec2 operator-(const vec2 & a) const {
            return vec2{this->x-a.x, this->y - a.y};
        }

        vec2 operator+(const vec2 & a) const {
            return vec2{this->x + a.x, this->y + a.y};
        }

        number operator*(const vec2 & a) {
            return (this->x*a.x + this->y*a.y);
        }

        vec2 operator*(const number & val) {
            return {this->x*val, this->y*val};
        }

        vec2 operator<<(const int & a) const {
            return vec2<number>{this->x*(1<<a), this->y*(1<<a)};
        }

        vec2 operator-() const {
            return vec2<number>{-this->x, -this->y};
        }

        vec2 operator>>(const int & a) const {
            return vec2<number>{this->x/(1<<a), this->y/(1<<a)};
        }

        bool operator==(const vec2 & rhs) const {
            return this->x==rhs.x && this->y==rhs.y;
        }
        bool operator!=(const vec2 & rhs) const {
            return !(*this==rhs);
        }

        vec2 & operator=(const vec2 & a) {
            this->x = a.x; this->y = a.y;
            return *this;
        }

    };

    //typedef vec2<float> vec2_f;
    typedef vec2<int32_t > vec2_32i;

//    extern template class vec2<float>;
//    extern template class vec2<int32_t>;

}
