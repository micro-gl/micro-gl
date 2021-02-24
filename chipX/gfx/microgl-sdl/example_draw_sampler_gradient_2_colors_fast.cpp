#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/Q.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/linear_gradient_2_colors.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;

int main() {
    using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= Canvas<Bitmap24, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
    using Texture24= sampling::texture<Bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    Resources resources{};
    auto * canvas = new Canvas24(W, H);;
    linear_gradient_2_colors<45> gradient{{255,0,0}, {0,0,255}};
    using number = float;
//    using number = Q<12>;

    auto render = [&]() -> void {
        static float t = 0;
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::None<>, false, number>(gradient, t, t, 400, 400);
    };

    example_run(canvas, render);
}

