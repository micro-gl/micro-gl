#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/ear_clipping_triangulation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

float t = 0;

template <typename number>
dynamic_array<vec2<number>> poly_rect() {
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {100, 300};
    return {p0, p1, p2, p3};
}

float b = 1;
template <typename number>
dynamic_array<vec2<number>> poly_2_x_monotone() {
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {200, 200};
    vertex p4 = {100, 300};

    return {p0, p1, p2, p3, p4};
}

template <typename number>
dynamic_array<vec2<number>> poly_tri() {
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p3 = {300, 100};
    vertex p4 = {100, 300};

    return {p0, p3, p4};
}

template <typename number>
dynamic_array<vec2<number>> poly_hole() {
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {100, 300};

    vertex p0_1 = {150,150};
    vertex p1_1 = {150, 200};
    vertex p2_1 = {200, 200};
    vertex p3_1 = {200, 150};
    int M=10;
    vertex p0_2 = {200,200};
    vertex p1_2 = {200, 300-M};
    vertex p2_2 = {300-M, 300-M};
    vertex p3_2 = {300-M, 200};

    return {p0, p1, p2, p3,
            p0_1, p1_1, p2_1,
            p0_2,
            p1_2, p2_2, p3_2,
            p0_2,
            p3_1,
            p0_1,p3};
}

template <typename number>
dynamic_array<vec2<number>> poly_hole3() {
    int M=10;
    return {
            {10,10},
            {400,10},
            {400,400},
            {10,400},

            {10,10},
            {20,20},
            {20,400-M},
            {400-M,400-M},
            {400-M,20},

            {20,20},
    };
}

template <typename number>
dynamic_array<vec2<number>> poly_hole4() {
    int M=10;
    return {
            {10,10},
            {400,10},
            {400,400},
            {10,400},

            {10,10},
            {20,20},
            {20,220},
            {400-0,220},
            {400-0,20},
            {20,20},
            {10,10},
    };
}

template <typename number>
dynamic_array<vec2<number>> poly_3() {
    return {
            {50,100},
            {100,50},
            {150,100},
            {200,50},
            {300,100},
            {400,50},
            {500,100},

            {500,200},
            {300-40,100+50},
            {300,100+100},
            {200,100+50},
            {150,100+100},
            {100,100+50},
            {50,100+100},

    };
}

int main() {
    using number = float;
//    using number = Q<12>;

    using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= Canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
    sampling::flat_color color_red{{255,0,0,255}};
    sampling::flat_color color_black{{0,0,0,255}};
    Resources resources{};

    auto * canvas = new Canvas24 (W, H);

    const auto render_polygon = [&](dynamic_array<vec2<number>> polygon) {
        using index = unsigned int;

        //polygon[1].x = 140 + 20 +  t;

        canvas->clear({255,255,255,255});

        using ear = microgl::tessellation::ear_clipping_triangulation<number>;

        triangles::indices type;
        dynamic_array<index> indices;
        dynamic_array<boundary_info> boundary_buffer;

        ear::compute(polygon.data(),
                     polygon.size(),
                     indices,
                     &boundary_buffer,
                     type
        );

        // draw triangles batch
        canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                matrix_3x3<number>::identity(),
                polygon.data(),
                (vec2<number> *)nullptr,
                indices.data(),
                boundary_buffer.data(),
                indices.size(),
                type,
                122);

        // draw triangulation
        canvas->drawTrianglesWireframe(
                {0,0,0,255},
                matrix_3x3<number>::identity(),
                polygon.data(),
                indices.data(),
                indices.size(),
                type,
                255);
    };

    const auto render = [&]() {
        t+=.05f;
//        std::cout << t << std::endl;
//        render_polygon(poly_rect<number>());
        render_polygon(poly_3<number>());
//        render_polygon(poly_hole<number>());
//        render_polygon(poly_hole3<number>());
//        render_polygon(poly_hole4<number>());
//        render_polygon(poly_rect<number>());
//        render_polygon(poly_tri<number>());
    };

    example_run(canvas, render);
}