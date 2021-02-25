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
//todo:: implement
using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
float t=0;

int main() {
    coder::coder_converter<coder::RGB888_ARRAY, coder::RGB888_PACKED_32> a{};
    using Canvas24= Canvas<Bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<Bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    using Texture32= sampling::texture<Bitmap<coder::RGBA8888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    using Texture8= sampling::texture<Bitmap<coder::RGB8>, sampling::texture_filter::NearestNeighboor>;
    using MaskingSampler= sampling::mask_sampler<Texture24, Texture24, masks::chrome_mode::red_channel>;
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
    MaskingSampler ms{tex_uv, tex_mask_2};

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                ms,
                0, 0, 300, 300);
    };

    example_run(canvas, render);
}
