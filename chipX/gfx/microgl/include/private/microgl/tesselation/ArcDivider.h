#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/array_container.h>
#include <cmath>

namespace tessellation {
#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268
#define TWO_PI    6.28318530718
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))
#define clamp_(v, e0, e1) (min_(max_(v,e0),e1))

    using index = unsigned int;
    using precision = unsigned char;
    using namespace microgl;

    class ArcDivider {
    public:

        explicit ArcDivider(bool DEBUG = false);;

        void compute(index radius,
                     int start_angle,
                     int end_angle,
                     precision precision,
                     index divisions,
                     bool clockwise,
                     array_container<vec2_32i> &output
                     );

        template<typename T>
        void swap(T& a, T& b);

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop