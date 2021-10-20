#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>

#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
float t=0;

int main() {
//    using number = Q<12>;
    using number = float;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;

    fast_radial_gradient<number> gradient{0.5, 0.5, 0.75};
    linear_gradient_2_colors<120> gradient2Colors{{255,0,255},
                                                  {255,0,0}};
    flat_color<> flatColor{{133,133,133, 255}};
    Canvas24 canvas(W, H);;

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});

    auto render = [&](void*, void*, void*) -> void {
        t+=0.17;
        canvas.clear({255,255,255,255});
        canvas.drawArc<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number,  number>(
                gradient2Colors,
                200, 200,
                50+0, 20,
                90+t, 90+60+t*2,
//                45, 90,
                true, // clock-wise
//                false, // clock-wise
                255);

    };
//    render();
    example_run(&canvas, render);
}
