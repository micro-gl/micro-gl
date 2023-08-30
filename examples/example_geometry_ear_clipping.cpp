#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/micro-tess/include/micro-tess/ear_clipping_triangulation.h>
#include <microgl/micro-tess/include/micro-tess/triangles.h>
#include <vector>
#include "microgl/micro-tess/include/micro-tess/static_array.h"

#define W 640*1
#define H 480*1

float t = 0;

template<typename item_type>
using static_arr = static_array<item_type, 100>;

template<typename item_type>
//using container = static_arr<item_type>;
using container = dynamic_array<item_type>;
//using container = std::vector<item_type>;

template <typename number>
container<vertex2<number>> poly_rect() {
    using il = std::initializer_list<vertex2<number>>;
    using vertex=vertex2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {100, 300};
    return il{p0, p1, p2, p3};
}

float b = 1;
template <typename number>
container<vertex2<number>> poly_2_x_monotone() {
    using il = std::initializer_list<vertex2<number>>;
    using vertex=vertex2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {200, 200};
    vertex p4 = {100, 300};

    return il{p0, p1, p2, p3, p4};
}

template <typename number>
container<vertex2<number>> poly_tri() {
    using il = std::initializer_list<vertex2<number>>;
    using vertex=vertex2<number>;
    vertex p0 = {100,100};
    vertex p3 = {300, 100};
    vertex p4 = {100, 300};

    return il{p0, p3, p4};
}

template <typename number>
container<vertex2<number>> poly_hole() {
    using il = std::initializer_list<vertex2<number>>;
    using vertex=vertex2<number>;
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

    return il{p0, p1, p2, p3,
            p0_1, p1_1, p2_1,
            p0_2,
            p1_2, p2_2, p3_2,
            p0_2,
            p3_1,
            p0_1,p3};
}

template <typename number>
container<vertex2<number>> poly_hole3() {
    using il = std::initializer_list<vertex2<number>>;
    int M=10;
    return il{
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
container<vertex2<number>> poly_hole4() {
    using il = std::initializer_list<vertex2<number>>;
    int M=10;
    return il{
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
container<vertex2<number>> poly_3() {
    using il = std::initializer_list<vertex2<number>>;
    return il{
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

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
    sampling::flat_color<> color_red{{255,0,0,255}};
    sampling::flat_color<> color_black{{0,0,0,255}};

    Canvas24 canvas(W, H);

    const auto render_polygon = [&](container<vertex2<number>> & polygon) {
        using index = unsigned int;

        //polygon[1].x = 140 + 20 +  t;
        microtess::triangles::indices type;
        container<index> indices;
        container<boundary_info> boundary_buffer;

        using ear = microtess::ear_clipping_triangulation<number,
                container<index>,
                container<boundary_info>>;

        ear::compute(polygon.data(),
                     polygon.size(),
                     indices,
                     &boundary_buffer,
                     type);

        // draw triangles batch
        canvas.clear({255,255,255,255});
        canvas.drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                matrix_3x3<number>::identity(),
                polygon.data(),
                (vertex2<number> *)nullptr,
                indices.data(),
                boundary_buffer.data(),
                indices.size(),
                type,
                122);

        // draw triangulation
        canvas.drawTrianglesWireframe(
                {0,0,0,255},
                matrix_3x3<number>::identity(),
                polygon.data(),
                indices.data(),
                indices.size(),
                type,
                255);
    };

    auto render = [&](void*, void*, void*) -> void {
        t+=.05f;
//        static auto poly = poly_rect<number>();
        static auto poly = poly_3<number>();
//        static auto poly = poly_hole<number>();
//        static auto poly = poly_hole3<number>();
//        static auto poly = poly_hole4<number>();
//        static auto poly = poly_tri<number>();

        render_polygon(poly);
    };

    example_run(&canvas, render, 100);

    return 0;
}