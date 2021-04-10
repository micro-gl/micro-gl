#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/lines_sampler.h>

#define W 640
#define H 640

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;
    using vertex = vec2<number>;

    const unsigned size = 10;
    vertex points[size] = {{0.25,0.25},
                           {0.5,0.25},
                           {0.75, 0.5},
                           {0.95, 0.25},
                           {0.95, 0.75},
                           {0.25, 0.75},
                           {0.25, 0.5},
                           {0.5, 0.75},
                           {0.5, 0.5},
                           {0.25, 0.25},
    };

    vertex points2[size] = {{0.20,0.5},
                           {1.-0.2,0.85}
    };

    lines_sampler<float> sampler{};

    sampler.updatePoints(points, size/1, 0.04);
//    sampler.updatePoints(points2, 2, 0.25);

    Canvas24 canvas(W, H);

    auto render = [&]() -> void {
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                sampler,
                0, 0, 500/2, 500/2);
    };

    example_run(&canvas, render, 100);
}

