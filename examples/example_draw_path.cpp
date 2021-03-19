#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

using microgl::tessellation::path;
float t = 0;

template <typename number>
path<number> path_star() {

    path<number> path{};
    path.lineTo({150, 150})
        .quadraticCurveTo({450, 0}, {450, 150})
        .lineTo({200,450})
        .lineTo({300,50})
        .lineTo({400,450})
        .closePath();
    return path;
}

template <typename number>
path<number> path_star_2() {
    path<number> path{};
    path.linesTo({{150, 150},
                  {450,150},
                  {200,450},
                  {300,50},
                  {400,450}})
            .moveTo({150/2, 150/2})
            .linesTo({{450/2,150/2},
                      {200/2,450/2},
                      {300/2,50/2},
                      {400/2,450/2}})
            .moveTo({150/10, 150/10})
            .linesTo({{450/10,150/10},
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
path<number> path_arc() {
    path<number> path{};
    int div=32; //4
    path.arc({200,200}, 100,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div).closePath();

    path.arc({250,200}, 50,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             true, div).closePath();
t+=0.82f;
//    t=120.539963f;//819999992f;//-0.01f;
////t=26.0399914;
    path.moveTo({150,150});
    path.arc({150+0,150}, 50+t-0,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div);//.closePath();

    return path;
}

template <typename number>
path<number> path_rects() {
    path<number> path{};
    path
        .rect(50, 50, 250, 250, false)
        .rect(50, 250, 550, 50, true)
        .rect(50, 450, 50, 50, true);
    return path;
}

template <typename number>
path<number> path_test() {
    path<number> path{};
    int div=32;
//    t+=0.01;
    t=137.999039f;
    path.linesTo({{100,100}, {300,100}, {300, 300}, {100,300}});
    vec2<number> start{22.0f, 150.0f-0.002323204};
    path.moveTo(start);
    path.linesTo({start, {300,120.002323204-t}, {300, 300}, {100,300}});
    path.moveTo({200, 200});
    path.lineTo({300,10});

    return path;
}

int main() {
    using number = float;
//    using number = double;
//    using number = Q<12>;
//    using number = Q<4>;

    using Canvas24= canvas<bitmap<RGB888_PACKED_32>>;
    sampling::flat_color<> color_red {{255,0,255,255}};

    Canvas24 canvas(W, H);

    auto render_path = [&](path<number> & path) {
        canvas.clear({255, 255, 255, 255});
        canvas.drawPathFill<blendmode::Normal, porterduff::None<>, false, false>(
                color_red,
                matrix_3x3<number>::identity(),
                path,
                tessellation::fill_rule::even_odd,
                tessellation::tess_quality::prettier_with_extra_vertices,
//            tessellation::tess_quality::better,
                255
        );

    };

    auto render = [&]() {
        static auto path = path_star_2<number>();
//        static auto path = path_star<number>();
//        static auto path = path_rects<number>();
//        static auto path = path_arc<number>();
//        static auto path = path_test<number>();

        render_path(path);
    };

    example_run(&canvas, render);
}


