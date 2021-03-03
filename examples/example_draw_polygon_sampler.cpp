#include "src/example.h"
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1
using namespace tessellation;

template <typename number>
dynamic_array<vec2<number>> poly_hole() {
    vec2<number> p0 = {100,100};
    vec2<number> p1 = {300, 100};
    vec2<number> p2 = {300, 300};
    vec2<number> p3 = {100, 300};

    vec2<number> p4 = {150,150};
    vec2<number> p7 = {150, 250};
    vec2<number> p6 = {250, 250};
    vec2<number> p5 = {250, 150};

//    return {p4, p5, p6, p7};
    return {p0, p1, p2, p3,   p4, p7, p6, p5, p4,p3};//,p5_,p4_};
}

template <typename number>
dynamic_array<vec2<number>> poly_diamond() {
    return {{300, 100}, {400, 300}, {300, 400}, {100,300}};
}

int main() {
    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
    using namespace microgl;
    using namespace microgl::sampling;

    linear_gradient_2_colors<90> gradient2Colors{{255,0,255},
                                                {255,0,0}};
    flat_color<> flatColor{{133,133,133, 255}};

    auto * canvas = new Canvas24(W, H);
    Resources resources{};

    auto render = [&]() -> void {
        static float t =0;

//        using number = float;
        using number = Q<15>;

//        auto polygon = poly_hole<number>();
        auto polygon = poly_diamond<number>();

        t+=.015f;
        polygon[3].x = 100 +  t;
        polygon[3].y = 300 -  t;
        canvas->clear({255,255,255,255});
        canvas->drawPolygon<
                polygons::hints::CONVEX,
//                polygons::hints::CONCAVE,
//            polygons::hints::SIMPLE,
//            polygons::hints::COMPLEX,
                blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, false>(
                gradient2Colors,
//                flatColor,
                matrix_3x3<number>::identity(),
                polygon.data(),
                polygon.size(),
                255);

    };

    example_run(canvas,
                render);

}
