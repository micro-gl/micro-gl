#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/white_noise_sampler.h>

#define W 640
#define H 640

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    white_noise_sampler<4> sampler{};
    Canvas24 canvas(W, H);


    auto render = [&](void*, void*, void*) -> void {
        static long long counter = 0;
        if(counter++%50==0)
            sampler.update();
        canvas.clear({255,0,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                sampler,
                0, 0, 250, 250);
    };

    example_run(&canvas, render, 100);

    return 0;
}

