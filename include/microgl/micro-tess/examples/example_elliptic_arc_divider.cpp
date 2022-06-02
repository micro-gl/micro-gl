#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#include <micro-tess/elliptic_arc_divider.h>
#include <micro-tess/static_array.h>
#include <micro-tess/dynamic_array.h>
#include <vector>

template<typename item_type>
using container = static_array<item_type, 100>;
//using container = dynamic_array<item_type>;
//using container = std::vector<item_type>;

int main() {
//    using number = float;
    using number = Q<15>;

    // microgl drawing setup START
    using Canvas24 = canvas<bitmap<coder::RGB888_PACKED_32>>;
    sampling::flat_color<> color_red{{255,0,0,255}};
    Canvas24 canvas(640, 480);
    // microgl drawing setup END

    const auto render_arc_internal = [&]() {
        // Algorithm START
        // inputs to algorithm
        container<vertex2<number>> output_points;
        number start_angle_rad = math::deg_to_rad(0.0f);
        number end_angle_rad = math::deg_to_rad(-45.0f);
        number rotation = math::deg_to_rad(45.0f);
        number radius_x = 50, radius_y = 75;
        number center_x = 200, center_y=200;
        unsigned int divisions_count = 32;
        const bool anti_clock_wise = false;
        // define algorithm
        using ellipse = microtess::elliptic_arc_divider<number, decltype(output_points)>;
        // compute algorithm
        ellipse::compute(
                output_points,    // output points
                center_x,         // x center of elliptic arc
                center_y,         // y center of elliptic arc
                radius_x,         // horizontal radius of elliptic arc
                radius_y,         // vertical radius of elliptic arc
                rotation,         // rotation about center
                start_angle_rad,  // starting angle radians
                end_angle_rad,    // end angle radians
                divisions_count,  // how many divisions
                anti_clock_wise   // anti clock-wise ?
        );
        // Algorithm END

        canvas.clear({255,255,255,255});
        for (uint ix = 0; ix < output_points.size() - 0; ++ix) {
            canvas.drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                    color_red, color_red,
                    output_points[ix].x,
                    output_points[ix].y,
                    number(4), number(1),
                    120);
        }

        canvas.drawWuLinePath(
                {0,0,0,255},
                output_points.data(),
                output_points.size(),
                false);

    };

    auto render = [&](void*, void*, void*) -> void {
        render_arc_internal();
    };

    example_run(&canvas, render);
}
