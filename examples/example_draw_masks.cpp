#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

int main() {
    using number = float;

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
    using Texture32= sampling::texture<bitmap<coder::RGBA8888_ARRAY>, sampling::texture_filter::NearestNeighboor>;

    Canvas24 canvas(W, H);
    Texture24 tex_mask, tex_1;
    Texture32 tex_0;
    float t=0;
    Resources resources{};

    auto img_0 = resources.loadImageFromCompressedPath("images/a.png");
    auto img_1 = resources.loadImageFromCompressedPath("images/charsprites.png");
    auto img_3 = resources.loadImageFromCompressedPath("images/bw.png");
//
    auto *bmp_0_native = new bitmap<coder::RGBA8888_ARRAY>(img_0.data, img_0.width, img_0.height);
    auto *bmp_1_native = new bitmap<coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height);
    auto *bmp_3_native = new bitmap<coder::RGB888_ARRAY>(img_3.data, img_3.width, img_3.height);

    tex_0.updateBitmap(bmp_0_native);
    tex_1.updateBitmap(bmp_1_native->convertToBitmap<coder::RGB888_PACKED_32>());
    tex_mask.updateBitmap(bmp_3_native->convertToBitmap<coder::RGB888_PACKED_32>());

    auto render = [&]() {

        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>>(
            tex_1,
            t, t, 300.0f, 300.0f,
            255);
        canvas.drawMask(masks::chrome_mode::red_channel,
//                tex_0,
                tex_mask,
                t, t, 300.0f, 300.0f);


    };

    example_run(&canvas, render);
}
