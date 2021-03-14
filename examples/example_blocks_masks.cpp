#include <iostream>
#include <chrono>
#include <SDL.h>
#include "src/Resources.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * sdl_texture;
Resources resources{};

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
using Canvas24= canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
Texture24 tex_uv, tex_mask;
Canvas24 * canvas_;
sampling::flat_color<> color_grey{{122,122,122,255}};
void loop();
void init_sdl(int width, int height);

template <typename number>
void render_blocks() {
    bool debug = 1;
    int block_size = W/8;//W/10;//2;//W/13;
    int count_blocks_horizontal = 1+((W-1)/block_size); // with integer ceil rounding
    int count_blocks_vertical = 1+((H-1)/block_size); // with integer ceil rounding
    auto * bitmap = new Bitmap24(block_size, block_size);
    auto * sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                    SDL_TEXTUREACCESS_STREAMING, block_size, block_size);

    canvas_->updateClipRect(0, 0, W, H);
    SDL_RenderClear(renderer);
    for (int iy = 0; iy < block_size*count_blocks_vertical; iy+=block_size) {
        for (int ix = 0; ix < block_size*count_blocks_horizontal; ix+=block_size) {
            canvas_->updateCanvasWindow(ix, iy, bitmap);
            canvas_->clear({255,255,255,255});
            canvas_->drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                    tex_uv,10, 10, 400, 400);
            canvas_->drawMask<number, number>(masks::chrome_mode::red_channel,
                             tex_mask,10, 10, 400.0f, 400.0f);

            SDL_Rect rect_source {0, 0, block_size, block_size};
            SDL_Rect rect_dest {ix, iy, block_size-debug, block_size-debug};
            SDL_UpdateTexture(sdl_texture,
                              &rect_source,
                              &canvas_->pixels()[0],
                              (canvas_->width()) * canvas_->sizeofPixel());
            SDL_RenderCopy(renderer, sdl_texture, &rect_source, &rect_dest);
        }
    }

    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(sdl_texture);

    delete bitmap;
}

void render() {
    render_blocks<float>();
    //render_blocks<Q<16>>();
}

int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 pixel_ Drawing", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STREAMING, width, height);

    auto img_2 = resources.loadImageFromCompressedPath("images/uv_512.png");
    auto img_3 = resources.loadImageFromCompressedPath("images/bw.png");
    auto bmp_uv_U8 = new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    auto bmp_3_native = new bitmap<coder::RGB888_ARRAY>(img_3.data, img_3.width, img_3.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
    tex_mask.updateBitmap(bmp_3_native->convertToBitmap<coder::RGB888_PACKED_32>());
    canvas_ = new Canvas24(width, height);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        render();
    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return int_ms.count();}

void loop() {
    bool quit = false;
    SDL_Event event;

    // 100 Quads
    int ms = render_test(TEST_ITERATIONS);
    std::cout << ms << std::endl;

    while (!quit)
    {
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                if( event.key.keysym.sym == SDLK_ESCAPE )
                    quit = true;
                break;
        }
//
        render();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
