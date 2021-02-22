#include <iostream>
#include <chrono>
#include <SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1
SDL_Window * window;
SDL_Renderer * renderer;
//SDL_Texture * sdl_texture;
Resources resources{};

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::Bilinear>;
Texture24 tex_uv;
Canvas24 * canvas;
sampling::flat_color color_grey{{0,0,122,255}};
void loop();
void init_sdl(int width, int height);
float t=0;

/*
 * NOTE:
 * SDL_RenderCopy is very slow and it's performance is for some reason not linear,
 * so don't mind performance in these blocks examples
 */

template <typename number>
void render_blocks() {
    //t+=0.01;
    bool debug = 1;
    int block_size = W/2;//W/10;//2;//W/13;
    int count_blocks_horizontal = 1+((W-1)/block_size); // with integer ceil rounding
    int count_blocks_vertical = 1+((H-1)/block_size); // with integer ceil rounding
    auto * bitmap = new Bitmap24(block_size, block_size);
    auto * sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                    SDL_TEXTUREACCESS_STREAMING, block_size, block_size);

    canvas->updateClipRect(0, 0, W, H);
    SDL_RenderClear(renderer);
    for (int iy = 0; iy < block_size*count_blocks_vertical; iy+=block_size) {
        for (int ix = 0; ix < block_size*count_blocks_horizontal; ix+=block_size) {
            canvas->updateCanvasWindow(ix, iy, bitmap);
            canvas->clear({255,255,255,255});
            canvas->drawRoundedRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                    tex_uv,
                    color_grey,
                    10+t, 10+t, 400+t, 400+t, 10, 1);

            SDL_Rect rect_source {0, 0, block_size, block_size};
            SDL_Rect rect_dest {ix, iy, block_size-debug, block_size-debug};
            SDL_UpdateTexture(sdl_texture,
                              &rect_source,
                              &canvas->pixels()[0],
                              (canvas->width()) * canvas->sizeofPixel());
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

    window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_512.png");
    auto bmp_uv_U8 = new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
    canvas = new Canvas24(width, height);
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
        render();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

