#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/line_linear_gradient.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    Canvas24 canvas(W, H);
    line_linear_gradient<number, 3, Canvas24::rgba> gradient{{0,1}, {1, 0}};

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.5f, {0,255,0});
    gradient.addStop(1.f, {0,0,255});

    auto render = [&](void*, void*, void*) -> void {
        static float t = 0;
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(gradient, t, t, 400, 400);
    };

    example_run(&canvas, render);
}
