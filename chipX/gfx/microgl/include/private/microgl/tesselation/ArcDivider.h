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
#define abs_(a) ((a)<0 ? -(a) : (a))
#define clamp_(v, e0, e1) (min_(max_(v,e0),e1))

    using index = unsigned int;
    using precision_t = unsigned char;
    using namespace microgl;

    class ArcDivider {
    public:

        explicit ArcDivider(bool DEBUG = false);;

        static void compute(
                     array_container<vec2_32i> &output,
                     index radius,
                     int center_x,
                     int center_y,
                     int start_angle_rad,
                     int end_angle_rad,
                     precision_t precision_angles,
                     index divisions=32,
                     bool anti_clockwise=false
                     );

        template<typename T>
        void swap(T& a, T& b);

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop