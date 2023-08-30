#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>

#define W 640*1
#define H 640*1

using namespace microgl::sampling;
float t=0;

int main() {
//    using number = Q<12>;
    using number = float;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;

    fast_radial_gradient<number> gradient{0.5, 0.5, 0.75};
    linear_gradient_2_colors<120> gradient2Colors{{255,0,255},
                                                  {255,0,0}};
    flat_color<> flatColor{{133,133,133, 255}};

    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_256.png");
    Canvas24 canvas(W, H);;
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});

    auto render = [&](void*, void*, void*) -> void {
        t+=0.001;
        canvas.clear({255,255,255,255});
        canvas.drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                tex_uv, gradient2Colors,
                200+0, 200+0,
                150+t, 10, 255);

    };

    example_run(&canvas, render);

    return 0;
}
