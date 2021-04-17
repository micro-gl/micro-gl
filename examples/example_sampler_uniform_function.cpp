#include "src/example.h"
#define MICROGL_USE_STD_MATH
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/uniform_function_sampler.h>

#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;
    using vertex = vec2<number>;

    const unsigned size = 100;
    vertex points[size];
    for (int ix = 0; ix < size; ++ix) {
        number xx = number(ix)/(size-1);
        number yy = microgl::math::sin(xx*2*2*microgl::math::pi<number>())*0.25f+0.5f;
        vertex v {xx, yy};
        points[ix]=v;
    }

//    const unsigned size = 3;
//    vertex points[size] = {{0.0, 0.}, {0.5,0.5}, {0.75, 0.5}};

    d1_function_sampler<number, rgba_t<8,8,8,8>, 2, 15> sampler{};

    sampler.updatePoints(points, size/1);
    sampler.color_fill = {0,0,0,255};
    sampler.color_stroke = {255,255,255,255};
    sampler.color_background = {0,255,255,0};

    float t = 0;
    Canvas24 canvas(W, H);

    auto render = [&]() -> void {
        t+=0.001;
        for (int ix = 0; ix < size; ++ix) {
            number xx = number(ix)/(size-1);
            number yy = microgl::math::sin(t+ xx*2*2*microgl::math::pi<number>())*0.25f+0.5f;
            vertex v {xx, yy};
            points[ix]=v;
        }

        sampler.updatePoints(points, size/1);

        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                sampler,
                0, 0, 500/2, 500/2);
    };

    example_run(&canvas, render,100);
}

