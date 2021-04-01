#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/checker_board.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
    using number = float;
//    using number = Q<12>;

    Canvas24 canvas(W, H);
    checker_board<> sampler{{0, 0, 0, 255},
                            {255, 255, 255, 255},
                            2, 2};

    checker_board_pot<1,1> sampler_pot{{0, 0, 0, 255},
                            {255, 255, 255, 255}};

    auto render = [&]() -> void {
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                sampler,
                0, 0, 300, 300);
    };

    example_run(&canvas, render);
}
