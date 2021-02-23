#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/Q.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/stroke_tessellation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

template <typename T>
dynamic_array<vec2<T>> path_diagonal() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {200, 200};
    vec2<T> p2 = {300, 300};

    return {p0, p1, p2};
}

template <typename T>
dynamic_array<vec2<T>> path_horizontal() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {500, 100};
    return {p0, p1};
}

template <typename T>
dynamic_array<vec2<T>> path_resh() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {300, 300};
    return {p0, p1, p2};
}

template <typename T>
dynamic_array<vec2<T>> path_2() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 400};
    vec2<T> p2 = {400, 100};
    vec2<T> p3 = {400, 300};

//    return {p0,p1,p2, p3, {50,50}};
    return {p0,p1,p2, p3};//, {250,320}};
    return {p0, p1, p2, p3};
}

template <typename T>
dynamic_array<vec2<T>> path_3() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {300, 300};
    vec2<T> p3 = {400, 300};

    return {p0, p1,p2};//, p3};
}


template <typename T>
dynamic_array<vec2<T>> path_line() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};

    return {p0, p1};
}

template <typename T>
dynamic_array<vec2<T>> path_tri() {
    vec2<T> p0 = {100, 100};
    vec2<T> p1 = {200, 100};
    vec2<T> p2 = {200, 200};
//    vec2<T> p3 = {220, 100};
    vec2<T> p3 = {300, 200};

    return {p0, p1, p2, p3};
//    return {p0, p1};
}

template <typename T>
dynamic_array<vec2<T>> path_rect() {
    vec2<T> p0 = {100, 200};
    vec2<T> p1 = {420, 100};
    vec2<T> p2 = {200, 100};
    vec2<T> p3 = {50, 50};

    return {p0, p1, p2, p3};
//    return {p0};
}

template <typename T>
dynamic_array<vec2<T>> path_rect2() {
    vec2<T> p0 = {100, 200};
    vec2<T> p1 = {100, 100};
    vec2<T> p2 = {200, 100};
    vec2<T> p3 = {200, 200};
    vec2<T> p4 = {100, 200};

    return {p0, p1, p2, p3, p4,p4};
//    return {p0, p1};
}
float t =0;
int main() {
    using number = float;
//    using number = double;
//    using number = Q<12>;
//    using number = Q<4>;

    using Canvas24= Canvas<Bitmap<coder::RGB888_PACKED_32>>;
    sampling::flat_color color_red{{255,0,0,255}};
    auto * canvas = new Canvas24(W, H);

    auto render_path = [&](const dynamic_array<vec2<number>> &path, number stroke_width, bool close_path) {
        using index = unsigned int;
        using stroke_tess = microgl::tessellation::stroke_tessellation<number>;

        //auto type = triangles::indices::TRIANGLES_STRIP;
        auto type = triangles::indices::TRIANGLES_STRIP_WITH_BOUNDARY;

        dynamic_array<index> indices;
        dynamic_array<vec2<number>> vertices;
        dynamic_array<boundary_info> boundary_buffer;
        t+=0.125f;

        stroke_tess::compute_with_dashes(
                stroke_width,
                close_path,
                tessellation::stroke_cap::butt,
//            tessellation::stroke_cap::square,
//            tessellation::stroke_cap::round,
                tessellation::stroke_line_join::round,
//            tessellation::stroke_line_join::bevel,
//            tessellation::stroke_line_join::miter_clip,
//            tessellation::stroke_line_join::miter,
//            tessellation::stroke_line_join::none,
                8,
                {50, 20}, t,
//            {50, 20}, t,
//            {0, 0}, 0,
                path.data(),
                path.size(),
                vertices,
                indices,
                type,
//            nullptr);
                &boundary_buffer);

        // draw triangles batch
        canvas->clear({255,255,255,255});
        canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                matrix_3x3<number>::identity(),
                vertices.data(),
                (vec2<number> *)nullptr,
                indices.data(),
                boundary_buffer.data(),
                indices.size(),
                type,
                127
        );

        return;
        // draw triangulation
        canvas->drawTrianglesWireframe(
                {0,0,0,255},
                matrix_3x3<number>::identity(),
                vertices.data(),
                indices.data(),
                indices.size(),
                type,
                255
        );

    };

    auto render = [&]() {
//        render_path(path_rect<number>(), 20.0f, false);
        render_path(path_rect<number>(), 5.0f, false);
//        render_path(path_rect2<number>(), 40.0f, true);
//        render_path(path_tri<number>(), 50.0f, false);
//        render_path(path_2<number>(), 20.0f, false);
//        render_path(path_line<number>(), 15.0f, false);
//        render_path(path_2<number>(), number(10.0f), true);
//        render_path(path_3<number>(), number(15.0f), false);
//        render_path(path_line<number>(), number(15.0f), false);
    };

    example_run(canvas, render);
}
