#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>

#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

int main() {

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24, CANVAS_OPT_32_BIT>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    Resources resources{};
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    Canvas24 canvas(W, H);
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    using number = float;

    auto render = [&]() -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                tex_uv,
                0, 0, 256, 256);
    };

    example_run(&canvas, render);
}
