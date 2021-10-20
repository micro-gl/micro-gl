#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
    using number = float;
//    using number = Q<12>;

    Canvas24 canvas(W, H);
    flat_color<> color_sampler{{255,122,0}};

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                color_sampler,
                0, 0, 400, 400);
    };

    example_run(&canvas, render);
}
