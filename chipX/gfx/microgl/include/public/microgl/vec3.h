#pragma once
#include <cstdint>

namespace microgl {

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


    typedef vec3<float> vec3_f;
    typedef vec3<uint8_t > vec3_8i;
    typedef vec3<uint16_t > vec3_16i;
    typedef vec3<int32_t> vec3_32i;
    typedef vec3<uint32_t > vec3_32ui;
}
