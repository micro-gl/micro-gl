#include "src/example.h"
#define MICROGL_USE_STD_MATH
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/1d_function_sampler.h>

#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;
    using vertex = vec2<number>;

    const unsigned size = 8;
    vertex points[size];

    for (int ix = 0; ix < size; ++ix) {
        number xx = number(ix)/(size-1);
        number yy = microgl::math::sin(xx*1*2*microgl::math::pi<number>())*0.25f+0.5f;
        vertex v {xx, yy};
        points[ix]=v;
    }

    d1_function_sampler<number> sampler{};

    sampler.updatePoints(points, size/1, 0.004);
//    sampler.updatePoints(points2, 2, 0.25);

    Canvas24 canvas(W, H);

    auto render = [&]() -> void {
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                sampler,
                0, 0, 500/2, 500/2);
    };

    example_run(&canvas, render, 100);
}

