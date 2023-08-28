//#define MICROGL_USE_STD_MATH
#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#include <micro-tess/path.h>
#include <micro-tess/dynamic_array.h>
#include <micro-tess/static_array.h>
#include <vector>

using microtess::path;
float offset = 0;

template<class item, class fake_allocator>
using static_array_with_fake_allocator = static_array<item, 10000, fake_allocator>;

template<typename number>
//using path_t = path<number, static_array_with_fake_allocator, std_rebind_allocator<>>;
//using path_t = path<number, std::vector, std_rebind_allocator<>>;
using path_t = path<number, dynamic_array, std_rebind_allocator<>>;

template <typename number>
path_t<number> path_star() {
    path_t<number> path{};
    path.lineTo({150, 150})
        .lineTo({450, 150})
        .lineTo({200,450})
        .lineTo({300,50})
        .lineTo({400,450})
        .closePath();
    return path;
}

template <typename number>
path_t<number> path_star_2() {
    using il = std::initializer_list<vertex2<number>>;
    path_t<number> path{};
    path.linesTo(il{{150, 150},
                  {450,150},
                  {200,450},
                  {300,50},
                  {400,450}})
            .moveTo({150/2, 150/2})
            .linesTo(il{{450/2,150/2},
                      {200/2,450/2},
                      {300/2,50/2},
                      {400/2,450/2}})
            .moveTo({150/10, 150/10})
            .linesTo(il{{450/10,150/10},
                      {200/10,450/10},
                      {300/10,50/10},
                      {400/10,450/10}})
            .rect(50, 50, 250, 250)
            .rect(50, 250, 550, 50, false)
            .rect(50, 450, 50, 50, false)
            .closePath();

    return path;
}

template <typename number>
path_t<number> path_arc_animation() {
    path_t<number> path{};
    int div=32; //4
    path.arc({200,200}, 100,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div).closePath();

    path.arc({250,200}, 50,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             true, div).closePath();
    offset+=0.82f;
//    t=120.539963f;//819999992f;//-0.01f;
////t=26.0399914;
    path.moveTo({150,150});
    path.arc({150+0,150}, 50+offset-0,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div);//.closePath();

    return path;
}

template <typename number>
path_t<number> path_rects() {
    path_t<number> path{};
    path.rect(50, 50, 250, 250, false)
        .rect(50, 250, 550, 50, true)
        .rect(50, 450, 50, 50, true);
    return path;
}

template <typename number>
path_t<number> path_test() {
    using il = std::initializer_list<vertex2<number>>;

    path_t<number> path{};
    int div=32;
//    t+=0.01;
    offset=137.999039f;
    path.linesTo(il{{100,100}, {300,100}, {300, 300}, {100,300}});
    vertex2<number> start{22.0f, 150.0f - 0.002323204};
    path.moveTo(start);
    path.linesTo(il{start, {300,120.002323204-offset}, {300, 300}, {100,300}});
    path.moveTo({200, 200});
    path.lineTo({300,10});

    return path;
}

int main() {
//    using number = float;
//    using number = double;
//    using number = Q<8>;
//    using number = Q<8>;
//    using number = Q<14, long long>;
    using number = Q<12, microgl::ints::int32_t, microgl::ints::int64_t>;
//    using number = Q<4, microgl::ints::int64_t>;
//    using number = Q<4, microgl::ints::int32_t, microgl::ints::int32_t, 0>;

    // microgl drawing setup START
    using Canvas24 = canvas<bitmap<RGB888_PACKED_32>>;
    using il = std::initializer_list<int>;
    sampling::flat_color<> color_red {{255,0,255,255}};
    sampling::flat_color<> color_green {{22,22,22,255}};
    Canvas24 canvas(640, 480);
    // microgl drawing setup END

    constexpr auto debug = false;
    number offset = 0;

    auto render_path = [&](path_t<number> & path) {
        offset+=number(0.125f);

        const auto & buffers= path.template tessellateStroke<>(
                number(12), // stroke width
//                tessellation::stroke_cap::butt,
                microtess::stroke_cap::round,
//                tessellation::stroke_cap::square,
//                tessellation::stroke_line_join::bevel,
//                tessellation::stroke_line_join::miter,
//                tessellation::stroke_line_join::miter_clip,
                microtess::stroke_line_join::round,
                10, // miter limit
                il{50, 50}, // dash container
                int(offset)); // dash offset

        canvas.clear({255, 255, 255, 255});
        canvas.drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_green,
                matrix_3x3<number>::identity(),
                buffers.output_vertices.data(),
                static_cast<vertex2<number> *>(nullptr),
                buffers.output_indices.data(),
                buffers.output_boundary.data(),
                buffers.output_indices.size(),
                buffers.output_indices_type,
                122);
        if(debug)
            canvas.drawTrianglesWireframe({0, 0, 0, 255},
                               matrix_3x3<number>::identity(),
                               buffers.output_vertices.data(),
                               buffers.output_indices.data(),
                               buffers.output_indices.size(),
                               buffers.output_indices_type,
                               255);
    };

    auto render = [&](void*, void*, void*) -> void {
        static auto path = path_star<number>();
//        static auto path = path_star_2<number>();
//        static auto path = path_rects<number>();
//        static auto path = path_arc_animation<number>();
//        static auto path = path_test<number>();

        render_path(path);
    };

    example_run(&canvas, render);
}


