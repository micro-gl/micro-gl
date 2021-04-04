#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

int main() {

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>>;
    using number = float;

    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_256.png");
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};

    Canvas24 canvas(W, H);

    auto render = [&]() -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
//                tex_uv,
                flat_color<>{{122, 122, 122, 255}},
                0, 0, 256, 256);
    };

    example_run(&canvas, render);
}
