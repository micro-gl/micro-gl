#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>

#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
    // using number = Q<12>;

    fast_radial_gradient<number> gradient{0.5, 0.5, 0.75};
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
                flatColor,
                gradient2Colors,
                10, 10, 300, 300,
                50, 10);
    };

    example_run(&canvas, render);
}

