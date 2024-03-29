#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
//    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_64_BIT>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    Canvas24 canvas(W, H);
    Texture24 tex_1, tex_2;

    auto img_1 = Resources::loadImageFromCompressedPath("images/charsprites.png");
//    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_512.png");
    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_512.png");

    tex_1.updateBitmap(new bitmap<coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height));
    tex_2.updateBitmap(new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height));

    auto render = [&](void*, void*, void*) -> void {
        using number = float;
//        using number = Q<15>;

        static float d =0;
        float G = 400;
        d+=0.2;

        canvas.clear(intensity<number>{1, 1, 1, 1});
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
        canvas.drawQuadrilateral<blendmode::Normal, porterduff::None<>, false, number, number>(
//                tex_1,
                tex_2,
                0.0f,               0.0f,     0.0f, 1.0f,
                G + 100.0f + d,     0.0f,       1.0f, 1.0f,
                G + 0.0f,                G,         1.0f, 0.0f,
                0.0f,                    G,         0.0f, 0.0f,
                255);

    };

    example_run(&canvas,
                render);


    return 0;
}
