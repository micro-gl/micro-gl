#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/circle_sampler.h>

#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    circle_sampler<number> sampler{};

    sampler.updatePoints({0.50,0.5},
                         0.5, 0.10);
    sampler.color_fill= {0, 0, 0, 255};
    sampler.color_background= {255, 255, 255, 0};
    sampler.color_stroke= {255, 255, 255, 255};

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                sampler,
                0, 0, 300, 300);
    };

    example_run(&canvas, render, 100);
}

