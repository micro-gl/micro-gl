#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/capsule_sampler.h>

#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;
//    using vertex = vec2<number>;

    capsule_sampler<number> sampler{};

//    sampler.updatePoints({0.20,0.5}, {0.20,0.5},
    sampler.updatePoints({0.20,0.5}, {1.-0.2,0.85},
                         0.25, 0.05);
    sampler.color_fill= {0, 0, 0, 255};
    sampler.color_stroke= {255, 255, 255, 255};
    // sampler.color_fill= {255, 255, 255, 0};

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                sampler,
                0, 0, 500/2, 500/2);
    };

    example_run(&canvas, render, 100);
}

