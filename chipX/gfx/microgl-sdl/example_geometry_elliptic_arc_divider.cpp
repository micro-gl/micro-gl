#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/elliptic_arc_divider.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

int main() {
//    using number = float;
    using number = Q<15>;

    using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= Canvas<Bitmap24>;
    sampling::flat_color color_red{{255,0,0,255}};

    auto * canvas = new Canvas24(W, H);

    const auto render_arc_internal = [&](
            const number& start_angle_rad,
            const number& end_angle_rad,
            const number& center_x, const number& center_y,
            const number& radius_x, const number& radius_y,
            const number& rotation,
            uint divisions_count) {
        using ellipse = microgl::tessellation::elliptic_arc_divider<number>;

        dynamic_array<vec2<number>> arc_points;

        ellipse::compute(
                arc_points,
                center_x,
                center_y,
                radius_x,
                radius_y,
                rotation,
                start_angle_rad,
                end_angle_rad,
                divisions_count,
//            false,
                true
        );

        canvas->clear({255,255,255,255});

        for (uint ix = 0; ix < arc_points.size()-0; ++ix) {
            canvas->drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                    color_red, color_red,
                    arc_points[ix].x,
                    arc_points[ix].y,
                    number(4), number(1),
                    120);
        }

        canvas->drawWuLinePath(
                {0,0,0,255},
                arc_points.data(),
                arc_points.size(),
                false);

    };

    number start_angle_rad = math::deg_to_rad(-45.0f);
    number end_angle_rad = math::deg_to_rad(360.0f);
    number rotation = math::deg_to_rad(45.0f);
    number radius_x = 50, radius_y = 75;
    number center_x = 200, center_y=200;

    const auto render = [&](){
        render_arc_internal(start_angle_rad, end_angle_rad,
                center_x, center_y, radius_x, radius_y,
                rotation,32);
    };

    example_run(canvas, render);
}
