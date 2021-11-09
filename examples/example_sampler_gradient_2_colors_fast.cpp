#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/linear_gradient_2_colors.h>

#define TEST_ITERATIONS 100
#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    linear_gradient_2_colors<45> gradient{{255,0,0}, {0,0,255}};

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                gradient,
                0, 0, 400, 400);
    };

    example_run(&canvas, render);
}

