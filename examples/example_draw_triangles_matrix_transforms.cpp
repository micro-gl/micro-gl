#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

float t = 0.0f;

int main() {
    using number  =float;
    using vertex = vec2<number>;
    using mat = matrix_3x3<number>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    Canvas24 canvas(W, H);
    sampling::flat_color<> color_red{{255,0,0,255}};

    vertex vertices[4] = {{0,0},
                          {100,0},
                          {100,100},
                          {0,100}};
    boundary_info boundary[2] = {create_boundary_info(true,true,false),
                                 create_boundary_info(false,true,true)};

    auto render = [&](void*, void*, void*) -> void {
        t += 0.001;
        auto t_number_angle = number(t);
        static float sine = 0.0f;
        sine = microgl::math::sin(t*2);
        auto number_scale = microgl::math::abs(number(sine) * 5);
        if (number_scale < 1.f) number_scale=1.f;

        mat identity = mat::identity();
        mat rotation = mat::rotation(t_number_angle);
        mat rotation_pivot = mat::rotation(t_number_angle, 50, 50,
                                           number_scale, number_scale / 2);

        mat translate = mat::translate(100.0f, 100);
        mat scale = mat::scale(number_scale, number_scale);
        mat shear_x = mat::shear_x(float(t));
        mat transform = translate * rotation_pivot;

        canvas.clear({255,255,255,255});
        canvas.drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                transform,
                vertices,
                (vec2<number> *) nullptr,
                nullptr,
                boundary,
                4,
                microtess::triangles::indices::TRIANGLES_FAN,
                150);
    };

    example_run(&canvas, render);
}
