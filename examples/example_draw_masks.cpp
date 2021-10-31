#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/samplers/texture.h>

#define W 640*1
#define H 480*1


int main() {
    using namespace microgl::sampling;
    using Canvas24  = canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24 = texture<bitmap<coder::RGB888_ARRAY>>;
    using Texture32 = texture<bitmap<coder::RGBA8888_ARRAY>>;
    using number = float;

    constexpr bool draw_first_mask = false;

    Canvas24 canvas(W, H);

    auto img_0 = Resources::loadImageFromCompressedPath("images/dog_32bit.png");
    auto img_1 = Resources::loadImageFromCompressedPath("images/charsprites.png");
    auto img_3 = Resources::loadImageFromCompressedPath("images/bw_8bits.png");

    Texture24 tex_1{new bitmap<coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height)};
    Texture24 tex_mask{new bitmap<coder::RGB888_ARRAY>(img_3.data, img_3.width, img_3.height)};
    Texture32 tex_mask_2{new bitmap<coder::RGBA8888_ARRAY>(img_0.data, img_0.width, img_0.height)};

    auto render = [&](void*, void*, void*) -> void {

        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>>(
                        tex_1,0.0f, 0.0f, 300.0f, 300.0f);

        if(draw_first_mask)
            canvas.drawMask(masks::chrome_mode::alpha_channel,
                            tex_mask_2,0.0f, 0.0f, 300.0f, 300.0f);
        else
            canvas.drawMask(masks::chrome_mode::red_channel,
                            tex_mask,0.0f, 0.0f, 300.0f, 300.0f);

    };

    example_run(&canvas, render);
}
