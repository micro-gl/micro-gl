#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/dynamic_array.h>
#include <microgl/math.h>

namespace tessellation {
#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268
#define TWO_PI    6.28318530718
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))

    using index = unsigned int;
    using precision_t = unsigned char;
    using math = microgl::math;
    using namespace microgl;

    template <typename number>
    class arc_divider {
    public:
        using vertex = vec2<number>;

        static void compute(
                     dynamic_array<vertex> &output,
                     number radius,
                     number center_x,
                     number center_y,
                     number start_angle_rad,
                     number end_angle_rad,
//                     precision_t precision_angles,
                     index divisions=32,
                     bool anti_clockwise=false
                     );

    private:
    };


}

#include "../../../../src/arc_divider.cpp"

#pragma clang diagnostic pop