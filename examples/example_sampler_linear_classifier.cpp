#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/linear_classifier_sampler.h>
#include <microgl/samplers/flat_color.h>

#define W 640
#define H 640

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    linear_classifier_sampler<number> sampler{};

    sampler.updatePoints({0.0,0.0},
                         {0.25, 0.50});
    sampler.color_left= {255, 255, 255, 255};
    sampler.color_right= {0, 0, 0, 255};

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                sampler,
                0, 0, 300, 300);
    };

    example_run(&canvas, render, 100);

    return 0;
}

