#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>

#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    fast_radial_gradient<number, 4, rgba_t<8,8,8,0>, precision::high> gradient{0.5, 0.5, 0.5};

    Canvas24 canvas(W, H);

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                gradient,
                0, 0, 400, 400);
    };

    auto render2 = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawQuadrilateral<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                gradient,
                0,          0,     0.0f, 1.0f,
                556,         0,       1.0f, 1.0f,
                256,         256,    1.0f, 0.0f,
                0,           256,    0.0f, 0.0f,
                255);
    };

//    auto & render3 = render2;
    auto & render3 = render;

    example_run(&canvas, render3);
}
