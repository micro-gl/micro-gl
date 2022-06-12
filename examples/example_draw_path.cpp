#define MICROGL_USE_STD_MATH

#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <vector>

#define W 640*1
#define H 640*1

using microtess::path;
float t = 0;

template<typename number>
//using path_t = path<number, std::vector>;
using path_t = path<number, dynamic_array>;

template <typename number>
path_t<number> path_star() {
    std::vector<int> a{100};
//    a.push_back()
    path_t<number> path{};
    path.lineTo({150, 150})
        .quadraticCurveTo({450, 0}, {450, 150})
        .lineTo({200,450})
        .lineTo({300,50})
        .lineTo({400,450})
        .closePath();
    return path;
}

template <typename number>
path_t<number> path_star_2() {
    path_t<number> path{};

    path.linesTo2(150, 150,
                  450,150,
                  200,450,
                  300,50,
                  400,450)
        .moveTo({150/2, 150/2})
        .linesTo2(450/2,150/2,
                  200/2,450/2,
                  300/2,50/2,
                  400/2,450/2)
        .moveTo({150/10, 150/10})
        .linesTo2(450/10,150/10,
                      200/10,450/10,
                      300/10,50/10,
                      400/10,450/10)
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
t+=1.82f;
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
path_t<number> path_rects() {
    path_t<number> path{};
    path
        .rect(50, 50, 250, 250, false)
        .rect(50, 250, 550, 50, true)
        .rect(50, 450, 50, 50, true);
    return path;
}

template <typename number>
path_t<number> path_test() {
    path_t<number> path{};
    using v = microtess::vec2<number>;
    int div=32;
//    t+=0.01;
    t=137.999039f;
    path.linesTo2(100,100, 300,100, 300,100, 300, 300, 100,300);
    v start{22.0f, 150.0f - 0.002323204};
    path.moveTo(start);
    path.linesTo3(start, v{300,120.002323204-t}, v{300, 300}, v{100,300});
    path.moveTo({200, 200});
    path.lineTo({300,10});

    return path;
}

int main() {
//    using number = float;
//    using number = double;
//    using number = Q<15, long long>;
    using number = Q<8, int32_t, int64_t, 1>;
//    using number = Q<2, int64_t>;
//    using number = Q<4, int32_t>;
//    using number = Q<12>;

    using Canvas24= canvas<bitmap<RGB888_PACKED_32>>;
    sampling::flat_color<> color_red {{255,0,255,255}};

    Canvas24 canvas(W, H);

    auto render_path = [&](path_t<number> & path) {
        canvas.clear({255, 255, 255, 255});
        canvas.drawPathFill<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, true>(
                color_red,
                matrix_3x3<number>::identity(),
                path,
                microtess::fill_rule::even_odd,
                microtess::tess_quality::prettier_with_extra_vertices,
//            tessellation::tess_quality::better,
                255
        );

    };

    auto render = [&](void*, void*, void*) -> void {
//        static auto path = path_star_2<number>();
//        static auto path = path_star<number>();
//        static auto path = path_rects<number>();
        auto path = path_arc_animation<number>();
//        static auto path = path_test<number>();

        render_path(path);
    };

    example_run(&canvas, render);
}


