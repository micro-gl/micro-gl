#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>

#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>,
                                        sampling::texture_filter::NearestNeighboor, false,
                                        sampling::texture_wrap::Clamp,
                                        sampling::texture_wrap::Repeat>;
    using number = Q<12>;
//    using number = float;

    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_256.png");
    Canvas24 canvas(W, H);;
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};

    auto render = [&]() -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, float, float>(
                tex_uv,
                10.0f, 10.0f, 300.0f, 300.0f,
                255,
                0.0f, 2.0f, 2.0f, 0.0f);
    };

    example_run(&canvas, render);
}
