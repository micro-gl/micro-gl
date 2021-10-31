#include "src/example.h"
#include "src/Resources.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>

#define W 640*1
#define H 480*1
using namespace microtess;
using namespace microgl::sampling;

template <typename number>
dynamic_array<microgl::vec2<number>> poly_hole() {
    using il = std::initializer_list<microgl::vec2<number>>;
    microgl::vec2<number> p0 = {100,100};
    microgl::vec2<number> p1 = {300, 100};
    microgl::vec2<number> p2 = {300, 300};
    microgl::vec2<number> p3 = {100, 300};

    microgl::vec2<number> p4 = {150,150};
    microgl::vec2<number> p7 = {150, 250};
    microgl::vec2<number> p6 = {250, 250};
    microgl::vec2<number> p5 = {250, 150};

//    return {p4, p5, p6, p7};
    return il{p0, p1, p2, p3,   p4, p7, p6, p5, p4,p3};//,p5_,p4_};
}

template <typename number>
dynamic_array<microgl::vec2<number>> poly_diamond() {
    using il = std::initializer_list<microgl::vec2<number>>;
    return il{{300, 100}, {400, 300}, {300, 400}, {100,300}};
}

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<15>;

    linear_gradient_2_colors<90> gradient2Colors{{255,0,255},
                                                {255,0,0}};
    flat_color<> flatColor{{133,133,133, 255}};

    Canvas24 canvas(W, H);

    auto render_polygon = [&](dynamic_array<microgl::vec2<number>> & polygon) -> void {
        static float t =0;

        t+=.015f;
        polygon[3].x = 100 +  t;
        polygon[3].y = 300 -  t;
        canvas.clear({255,255,255,255});
        canvas.drawPolygon<
//                polygons::hints::CONVEX,
                polygons::hints::SIMPLE,
//                polygons::hints::CONCAVE,
//                polygons::hints::X_MONOTONE,
//                polygons::hints::Y_MONOTONE,
//                 polygons::hints::SELF_INTERSECTING,
                blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, false>(
                gradient2Colors,
//                flatColor,
                matrix_3x3<number>::identity(),
                polygon.data(),
                polygon.size(),
                255);

    };

    auto render = [&](void*, void*, void*) -> void {
//        static auto poly = poly_hole<number>();
        static auto polygon = poly_diamond<number>();

        render_polygon(polygon);

    };

    example_run(&canvas,
                render);

}
