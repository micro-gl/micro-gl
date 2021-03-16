#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGBA8888_PACKED_32.h>
#include <microgl/porter_duff/FastSourceOverOnOpaque.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/blend_modes/Multiply.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;


int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>>;
    using Texture32= sampling::texture<bitmap<coder::RGBA8888_ARRAY>>;

    Resources resources{};
    float t=0;

    auto img_0 = resources.loadImageFromCompressedPath("images/dog_32bit.png");
    auto img_1 = resources.loadImageFromCompressedPath("images/charsprites.png");
//
    auto * canvas = new Canvas24(W, H);
    Texture24 tex_1{new bitmap<coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height)};
    Texture32 tex_0{new bitmap<coder::RGBA8888_ARRAY>(img_0.data, img_0.width, img_0.height)};

    auto render = [&]() -> void {
        canvas->clear({0,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque>(
                tex_1,
                t, t, 300.0f, 300.0f,
                255);
        canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque>(
                tex_0,
                t, t, 300.0f, 300.0f,
                255);
    };

    example_run(canvas,
                render);
}
