#include <iostream>
#include "src/example.h"
#include "src/Resources.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1
#define BLOCK_SIZE W/4

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;

int main() {
    using number = float;
//    using number = <Q<16>;

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::Bilinear>;
    Texture24 tex_uv;
    sampling::flat_color<> color_grey{{122,122,122,255}};

    auto img_2 =Resources::loadImageFromCompressedPath("images/uv_512.png");
    auto bmp_uv_U8 = new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
    Canvas24 canvas(BLOCK_SIZE, BLOCK_SIZE);
    canvas.updateClipRect(0, 0, W, H);

    auto render_blocks = [&](SDL_Renderer * renderer) -> void {
        bool debug = 1;
        int block_size = BLOCK_SIZE;
        int count_blocks_horizontal = 1+((W-1)/block_size); // with integer ceil rounding
        int count_blocks_vertical = 1+((H-1)/block_size); // with integer ceil rounding
        auto * sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                               SDL_TEXTUREACCESS_STREAMING, block_size, block_size);

        SDL_RenderClear(renderer);
        for (int iy = 0; iy < block_size*count_blocks_vertical; iy+=block_size) {
            for (int ix = 0; ix < block_size*count_blocks_horizontal; ix+=block_size) {
                canvas.updateCanvasWindow(ix, iy);
                canvas.clear({255,255,255,255});
                canvas.drawRoundedRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                        tex_uv,
                        color_grey,
                        10+0, 10+0, 400+0, 400+0, 10, 1);

                SDL_Rect rect_source {0, 0, block_size, block_size};
                SDL_Rect rect_dest {ix, iy, block_size-debug, block_size-debug};
                SDL_UpdateTexture(sdl_texture,
                                  &rect_source,
                                  &canvas.pixels()[0],
                                  (canvas.width()) * canvas.sizeofPixel());
                SDL_RenderCopy(renderer, sdl_texture, &rect_source, &rect_dest);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(sdl_texture);
    };

    auto render = [&](void*, SDL_Renderer * renderer, void*) -> void {
        render_blocks(renderer);
    };

    example_run(&canvas, W, H, render, 100, true);
}
