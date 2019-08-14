#pragma once

#include <cstdint>

typedef struct {
    uint8_t a1,a2,a3;
} U8A;

typedef struct {
    uint8_t r, g, b, a;
} color_t;

struct color_f_t {
    float r, g, b, a;

    color_f_t& operator*(const float & val)
    {
        r *= val;
        g *= val;
        b *= val;
        return *this;
    }

};

template<typename T>
struct vec2 {

    T x, y;

    vec2() {

    }

    vec2(const T & x_, const T & y_) {
        x = x_;
        y = y_;
    }

    template<typename F>
    vec2(const vec2<F> & a) {
        this->x = static_cast<T>(a.x);
        this->y = static_cast<T>(a.y);
    }


//    template<typename F>
//    explicit vec2(const vec2<F> & a) {
//        this->x = static_cast<T>(a.x);
//        this->y = static_cast<T>(a.y);
//    }

    vec2 operator-(const vec2 & a) const {
        return vec2{this->x-a.x, this->y - a.y};
    }

    vec2 operator+(const vec2 & a) const {
        return vec2{this->x + a.x, this->y + a.y};
    }

    vec2 operator-(const vec2 & a) {
        return vec2{this->x-a.x, this->y - a.y};
    }

    T operator*(const vec2 & a) {
        return (this->x*a.x + this->y*a.y);
    }

//    vec2<T> operator*(const int & a) const {
//        return vec2<T>{this->x*a, this->y*a};
//    }

    template<typename F>
    vec2<T> operator*(const F & a) const {
        return vec2<T>{this->x*a, this->y*a};
    }

    vec2<T> operator<<(const int & a) const {
        return vec2<T>{this->x*(1<<a), this->y*(1<<a)};
    }

    vec2<T> operator>>(const int & a) const {
        return vec2<T>{this->x/(1<<a), this->y/(1<<a)};
    }

    bool operator==(const vec2 & rhs) const {
        return this->x==rhs.x &&
                this->y==rhs.y;
    }

    template<typename F>
    vec2<T> & operator=(const vec2<F> & a) {
        this->x = static_cast<T>(a.x);
        this->y = static_cast<T>(a.y);
        return *this;
    }

};

template<typename T>
struct vec3 {
    T x, y, z;

    vec3 &operator=(const vec3 & a) {
        x=a.x;
        y=a.y;
        z=a.z;

        return *this;
    }
};

template<typename T>
struct vec4 {
    T x, y, z, w;
};

typedef vec2<float> vec2_f;
typedef vec2<uint8_t > vec2_8i;
typedef vec2<uint16_t > vec2_16i;
typedef vec2<uint32_t > vec2_32ui;
typedef vec2<int32_t > vec2_32i;

typedef vec3<float> vec3_f;
typedef vec3<uint8_t > vec3_8i;
typedef vec3<uint16_t > vec3_16i;
typedef vec3<int32_t> vec3_32i;
typedef vec3<uint32_t > vec3_32ui;