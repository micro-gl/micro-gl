#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    using Texture32= sampling::texture<bitmap<coder::RGBA8888_ARRAY>>;
//    using number = Q<12>;
    using number = float;

    Resources resources{};
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto img_1 = resources.loadImageFromCompressedPath("images/dog_32bit.png");

    bitmap<coder::RGB888_ARRAY> a(img_2.data, img_2.width, img_2.height);
    bitmap<coder::RGB888_ARRAY> a_copy(a);
    bitmap<coder::RGB888_ARRAY> a_move(bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height));
    bitmap<coder::RGB888_ARRAY> a_move_2 = microgl::traits::move(a);//bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    a_move_2 = microgl::traits::move(a);//bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
//    bitmap<coder::RGB888_ARRAY> a_move2(microgl::traits::move(a));

    Canvas24 canvas(W, H);
//    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    Texture24 tex_uv{&a_move_2};
    Texture32 tex_uv_32{new bitmap<coder::RGBA8888_ARRAY>(img_1.data, img_1.width, img_1.height)};

    auto render = [&]() -> void {
        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                tex_uv,
//                tex_uv_32,
                0, 0, 300, 300);
    };

    example_run(&canvas, render);
}
