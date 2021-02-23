#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>

#define TEST_ITERATIONS 1
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

    fast_radial_gradient<float> gradient{0.5, 0.5, 0.75};
    linear_gradient_2_colors<120> gradient2Colors{{255,0,255}, {255,0,0}};
    flat_color flatColor{{133,133,133, 255}};
    Resources resources{};
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto * canvas = new Canvas24(W, H);;
    Texture24 tex_uv{new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
        canvas->drawRoundedRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                tex_uv,
            gradient2Colors,
//                flatColor,
                10+t, 10+t, 400+t, 400+t,
                50, 10);
    };

    example_run(canvas, TEST_ITERATIONS, render);
}

