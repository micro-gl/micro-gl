#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define W 640*1
#define H 640*1
//#define USE_TEXTURE

using namespace microgl::sampling;

int main() {

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;

#ifdef USE_TEXTURE
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>>;
    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_256.png");
    Texture24 sampler{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
#else
    flat_color<rgba_t<8,8,8,8>> sampler{{122, 122, 122, 255}};
#endif

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                sampler,
                0, 0, 256, 256);
    };

    example_run(&canvas, render);
}
