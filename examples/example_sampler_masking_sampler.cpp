#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGB8.h>
#include <microgl/pixel_coders/coder_converter.h>
#include <microgl/pixel_coders/coder_rgba.h>
#include <microgl/samplers/mask_sampler.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
float t=0;

int main() {
    using Canvas24= Canvas<Bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<Bitmap<coder::RGB888_ARRAY>>;
    using Texture32= sampling::texture<Bitmap<coder::RGBA8888_ARRAY>>;
    using MaskingSampler1= sampling::mask_sampler<masks::chrome_mode::red_channel, Texture24, Texture24>;
    using MaskingSampler2= sampling::mask_sampler<masks::chrome_mode::alpha_channel_inverted, Texture24, Texture32>;

//    using number = Q<12>;
    using number = float;

    Resources resources{};
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto img_mask_1 = resources.loadImageFromCompressedPath("images/a.png");
    auto img_mask_2 = resources.loadImageFromCompressedPath("images/bw2.png");
    auto * canvas = new Canvas24(W, H);
    Texture24 tex_uv{new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    Texture32 tex_mask_1{new Bitmap<coder::RGBA8888_ARRAY>(img_mask_1.data, img_mask_1.width, img_mask_1.height)};
    Texture24 tex_mask_2{new Bitmap<coder::RGB888_ARRAY>(img_mask_2.data, img_mask_2.width, img_mask_2.height)};
    MaskingSampler1 ms{tex_uv, tex_mask_2};
    MaskingSampler2 ms2{tex_uv, tex_mask_1};

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
//                ms,
                ms2,
                0, 0, 300, 300);
    };

    example_run(canvas, render);
}
