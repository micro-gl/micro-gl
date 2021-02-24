#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/Q.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/fast_radial_gradient.h>

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
    fast_radial_gradient<float, 10, precision::high> gradient{0.5, 0.5, 0.5};
    using number = float;
//    using number = Q<12>;

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});

    auto render = [&]() -> void {
        static float t = 0;
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::None<>, false, number>(gradient, t, t, 400, 400);
    };

    auto render2 = [&]() -> void {
        static float t = 0;
        canvas->clear({255,255,255,255});
        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None<>, false, float>(
                gradient,
                0.0f,               0.0f,     0.0f, 1.0f,
                256 + 100.0f + t,     0.0f,       1.0f, 1.0f,
                256 + 0.0f,           256,         1.0f, 0.0f,
                0.0f,                 256,         0.0f, 0.0f,
                255);
    };

    auto & render3 = render;

    example_run(canvas, render3);
}
