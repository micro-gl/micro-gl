#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/monotone_polygon_triangulation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

template <typename number>
dynamic_array<vec2<number>> poly_rect() {
    using il = std::initializer_list<vec2<number>>;
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {100, 300};
    return il{p0, p1, p2, p3};
}

template <typename number>
dynamic_array<vec2<number>> poly_tri() {
    using il = std::initializer_list<vec2<number>>;
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p3 = {300, 100};
    vertex p4 = {100, 300};

    return il{p0, p3, p4};
}

template <typename number>
dynamic_array<vec2<number>> poly_1_x_monotone() {
    using il = std::initializer_list<vec2<number>>;
    return il{
            {50,100},
            {100,50},
            {150,100},
            {200,50},
            {300,100},
            {400,50},
            {500,100},

            {500,200},
            {400,100+50},
            {300,100+100},
            {200,100+50},
            {150,100+100},
            {100,100+50},
            {50,100+100},

    };
}

template <typename number>
dynamic_array<vec2<number>> poly_1_y_monotone() {
    using il = std::initializer_list<vec2<number>>;
    return il{
            {100, 50},
            {50, 100},
            {100, 150},
            {50,200},
            {100,300},
            {50,400},
            {100,500},

            {200,500},
            {100+50,400},
            {100+100,300},
            {100+50,200},
            {100+100,150},
            {100+50,100},
            {100+100,50},

    };
}

template <typename number>
dynamic_array<vec2<number>> poly_2_x_monotone() {
    using il = std::initializer_list<vec2<number>>;
    return il{
            {50,200},
            {50,100},
            {100,150},
            {150,100},
            {200,190},
            {250,100},
            {300,150},
            {350,100},
            {400,150},
            {400,200},

    };
}

template <typename number>
dynamic_array<vec2<number>> poly_2_y_monotone() {
    using il = std::initializer_list<vec2<number>>;
    return il{
            {200,50},
            {100,50},
            {150,100},
            {100,150},
            {190,200},
            {100,250},
            {150,300},
            {100,350},
            {150,400},
            {200,400},

    };
}

int main() {
    using number = float;
//    using number = Q<12>;

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
    sampling::flat_color<> color_red{{255,0,0,255}};
    sampling::flat_color<> color_black{{0,0,0,255}};
    Canvas24 canvas(W, H);
    float t = 0;

    auto render_polygon = [&](const dynamic_array<vec2<number>> & polygon, bool x_monotone_or_y) {
        using index = unsigned int;
        using mat = matrix_3x3<number>;

//    polygon[1].x = 140 + 20 +  t;

        canvas.clear({255,255,255,255});

        using mpt = tessellation::monotone_polygon_triangulation<number>;

        triangles::indices type;
        dynamic_array<index> indices;
        dynamic_array<boundary_info> boundary_buffer;

        mpt::compute(polygon.data(),
                     polygon.size(),
                     x_monotone_or_y ? mpt::monotone_axis::x_monotone : mpt::monotone_axis::y_monotone,
                     indices,
                     &boundary_buffer,
                     type
        );

        mat transform = matrix_3x3<number>::scale(1,1);
        // draw triangles batch
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, false>(
                color_red,
                transform,
                polygon.data(),
                (vec2<number> *)nullptr,
                indices.data(),
                boundary_buffer.data(),
                indices.size(),
                type,
                255);

        // draw triangulation
        canvas.drawTrianglesWireframe(
                {0,0,0,255},
                transform,
                polygon.data(),
                indices.data(),
                indices.size(),
                type,
                255);
    };

    auto render = [&]() {
        static auto poly = poly_1_x_monotone<number>();
//        static auto poly = poly_2_y_monotone<number>();

        render_polygon(poly, true);
//        render_polygon(poly, false);
    };

    example_run(&canvas, render);

}
