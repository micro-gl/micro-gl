#pragma once

#include <microgl/vec2.h>
#include <microgl/dynamic_array.h>
#include <microgl/math.h>

namespace tessellation {

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
                     int divisions=32,
                     bool anti_clockwise=false
                     );

    private:
    };


}

#include "arc_divider.cpp"