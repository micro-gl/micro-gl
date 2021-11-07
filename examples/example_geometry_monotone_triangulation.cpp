#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/micro-tess/monotone_polygon_triangulation.h>
#include <microgl/micro-tess/static_array.h>
#include <vector>

#define W 640*1
#define H 480*1

template<typename item_type>
using static_arr = static_array<item_type, 800>;

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
container<vertex2<number>> poly_1_x_monotone() {
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
            {400,100+50},
            {300,100+100},
            {200,100+50},
            {150,100+100},
            {100,100+50},
            {50,100+100},

    };
}

template <typename number>
container<vertex2<number>> poly_1_y_monotone() {
    using il = std::initializer_list<vertex2<number>>;
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
container<vertex2<number>> poly_2_x_monotone() {
    using il = std::initializer_list<vertex2<number>>;
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
container<vertex2<number>> poly_2_y_monotone() {
    using il = std::initializer_list<vertex2<number>>;
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

    auto render_polygon = [&](const container<vertex2<number>> & polygon, bool x_monotone_or_y) {
        using index = unsigned int;
        using mat = matrix_3x3<number>;

//    polygon[1].x = 140 + 20 +  t;
        microtess::triangles::indices type;
        container<index> indices;
        container<boundary_info> boundary_buffer;


        using mpt = microtess::monotone_polygon_triangulation<number, container<index>, container<boundary_info>>;

        mpt::compute(polygon.data(),
                     polygon.size(),
                     x_monotone_or_y ? mpt::monotone_axis::x_monotone : mpt::monotone_axis::y_monotone,
                     indices,
                     &boundary_buffer,
                     type
        );

        mat transform = matrix_3x3<number>::scale(1,1);
        // draw triangles batch
        canvas.clear({255,255,255,255});
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, false>(
                color_red,
                transform,
                polygon.data(),
                (vertex2<number> *)nullptr,
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

    auto render = [&](void*, void*, void*) -> void {
        render_polygon(poly_1_x_monotone<number>(), true);
//        render_polygon(poly_2_y_monotone<number>(), false);
//        render_polygon(poly_rect<number>(), true);
//        render_polygon(poly_tri<number>(), false);
    };

    example_run(&canvas, render);

}
