#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/samplers/void_sampler.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>

#define W 640*1
#define H 640*1

using namespace microgl::sampling;
float t=0;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
    using number = float;
    // using number = Q<12>;

    fast_radial_gradient<float> gradient{0.5, 0.5, 0.75};
    linear_gradient_2_colors<120> gradient2Colors{{255,0,255}, {255,0,0}};
    flat_color<> flatColor{{133,133,133, 255}};

    Canvas24 canvas(W, H);;

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255/1,255,255,255});
        canvas.drawRoundedRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
//                flatColor,
//                gradient,
                void_sampler{},
                void_sampler{},
//                gradient2Colors,
//                flatColor,
                10+t, 10+t, 300+t, 300+t,
                50, 10);
    };

    example_run(&canvas, render);

    return 0;
}

