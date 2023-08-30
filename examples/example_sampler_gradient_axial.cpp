#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/axial_linear_gradient.h>

#define TEST_ITERATIONS 100
#define W 640
#define H 640

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    Canvas24 canvas(W, H);
    axial_linear_gradient<135, 3, Canvas24::rgba> gradient;

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.5f, {0,255,0});
    gradient.addStop(1.f, {0,0,255});

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                gradient,
                0, 0, 400, 400);
    };

    example_run(&canvas, render);

    return 0;
}
