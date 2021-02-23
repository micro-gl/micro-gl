#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
float t=0;

int main() {
    using Canvas24= Canvas<Bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
    using Texture24= sampling::texture<Bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
//    using number = Q<12>;
    using number = float;

    auto * canvas = new Canvas24(W, H);;
    flat_color color_sampler{{255,122,0}};

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                color_sampler, t, t, 400, 400);
    };

    example_run(canvas, TEST_ITERATIONS, render);
}
