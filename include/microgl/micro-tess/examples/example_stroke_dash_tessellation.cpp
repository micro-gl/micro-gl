#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/math/Q.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>

#include <micro-tess/stroke_tessellation.h>
#include <micro-tess/static_array.h>
#include <vector>

template<typename item>
//using container = dynamic_array<item>;
//using container = std::vector<item>;
using container = static_array<item, 10000>;

template <typename number>
container<vertex2<number>> path_diagonal() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {200, 200};
    vertex2<number> p2 = {300, 300};

    return il{p0, p1, p2};
}

template <typename number>
container<vertex2<number>> path_horizontal() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {300, 100};
    vertex2<number> p2 = {500, 100};
    return il{p0, p1};
}

template <typename number>
container<vertex2<number>> path_resh() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {300, 100};
    vertex2<number> p2 = {300, 300};
    return il{p0, p1, p2};
}

template <typename number>
container<vertex2<number>> path_2() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {300, 400};
    vertex2<number> p2 = {400, 100};
    vertex2<number> p3 = {400, 300};

//    return {p0,p1,p2, p3, {50,50}};
    return il{p0,p1,p2, p3};//, {250,320}};
    return il{p0, p1, p2, p3};
}

template <typename number>
container<vertex2<number>> path_3() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {300, 100};
    vertex2<number> p2 = {300, 300};
    vertex2<number> p3 = {400, 300};

    return il{p0, p1,p2};//, p3};
}


template <typename number>
container<vertex2<number>> path_line() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {300, 100};

    return il{p0, p1};
}

template <typename number>
container<vertex2<number>> path_tri() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 100};
    vertex2<number> p1 = {200, 100};
    vertex2<number> p2 = {200, 200};
//    vertex2<T> p3 = {220, 100};
    vertex2<number> p3 = {300, 200};

    return il{p0, p1, p2, p3};
//    return {p0, p1};
}

template <typename number>
container<vertex2<number>> path_1() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 200};
    vertex2<number> p1 = {420, 100};
    vertex2<number> p2 = {200, 100};
    vertex2<number> p3 = {50, 50};

    return il{p0, p1, p2, p3};
}

template <typename number>
container<vertex2<number>> path_rect2() {
    using il = std::initializer_list<vertex2<number>>;
    vertex2<number> p0 = {100, 200};
    vertex2<number> p1 = {100, 100};
    vertex2<number> p2 = {200, 100};
    vertex2<number> p3 = {200, 200};
    vertex2<number> p4 = {100, 200};

    return il{p0, p1, p2, p3, p4,p4};
}

int main() {
    using number = float;
//    using number = double;
//    using number = Q<12>;
//    using number = Q<4>;

    // microgl drawing setup START
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_64_BIT_FREE>;
    sampling::flat_color<> color_red{{255,0,0,255}};
    Canvas24 canvas(640, 480);
    // microgl drawing setup END

    float t = 0;
    auto render_path = [&](const container<vertex2<number>> & path, number stroke_width, bool close_path) {
        using index = unsigned int;

        t+=0.28;

        // Algorithm START
        // output vertices attributes
        container<vertex2<number>> output_vertices;
        // output indices
        container<index> output_indices;
        // output boundary info
        container<boundary_info> output_boundary_buffer;
        // output triangles type
        microtess::triangles::indices output_triangles_type;
        // define algorithm
        using stroke_tess = microtess::stroke_tessellation<number,
                container<vertex2<number>>, container<index>,
                container<boundary_info>>;
        // compute algorithm
        stroke_tess::compute_with_dashes(
                stroke_width,
                close_path,
//                microtess::stroke_cap::butt,
//            microtess::stroke_cap::square,
                microtess::stroke_cap::round,
                microtess::stroke_line_join::round,
//            microtess::stroke_line_join::bevel,
//            microtess::stroke_line_join::miter_clip,
//            microtess::stroke_line_join::miter,
//            microtess::stroke_line_join::none,
                8,
                std::initializer_list<int>{50, 50}, t,
                path.data(),
                path.size(),
                output_vertices,
                output_indices,
                output_triangles_type,
//            nullptr);
                &output_boundary_buffer);

        // draw triangles batch
        canvas.clear({255,255,255,255});
        canvas.drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                matrix_3x3<number>::identity(),
                output_vertices.data(),
                (vertex2<number> *)nullptr,
                output_indices.data(),
                output_boundary_buffer.data(),
                output_indices.size(),
                output_triangles_type,
                255/2
        );

//        return;
        canvas.drawTrianglesWireframe(
                {0,0,0,255},
                matrix_3x3<number>::identity(),
                output_vertices.data(),
                output_indices.data(),
                output_indices.size(),
                output_triangles_type,
                255/4
        );

    };

    auto render = [&](void*, void*, void*) -> void {
        static auto path = path_1<number>();
//        static auto path = path_rect2<number>();
//        static auto path = path_tri<number>();
//        static auto path = path_2<number>();
//        static auto path = path_line<number>();
//        static auto path = path_3<number>();

        render_path(path, 40.0f, false);
//        render_path(path, 40.0f, false);
//        render_path(path, 40.0f, true);
//        render_path(path, 50.0f, false);
//        render_path(path, 20.0f, false);
//        render_path(path, 15.0f, false);
//        render_path(path, number(10.0f), true);
//        render_path(path, number(15.0f), false);
//        render_path(path, number(15.0f), false);
    };

    example_run(&canvas, render);
}