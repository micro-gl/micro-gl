#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/color.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/porter_duff/FastSourceOverOnOpaque.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/blend_modes/Multiply.h>
#include <microgl/Bitmap.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;

Canvas24 * canvas;
Texture24 tex_mask, tex_1, tex_2;
float t=0;
Resources resources{};
using namespace microgl::color;
void loop();
void init_sdl(int width, int height);

inline void render() {
    //t+=-0.01;
    canvas->clear({255,255,255,255});
    canvas->drawRect<blendmode::Normal, porterduff::None<>>(
            tex_1,
            t, t, 300.0f, 300.0f,
            255);
//    canvas->drawQuad<blendmode::Normal, porterduff::None>(
//            color::colors::RED, -0, -0, 300, 300, 255);
    canvas->drawMask(masks::chrome_mode::red_channel,
            tex_mask,
            t, t, 300.0f, 300.0f);


}

int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);

    canvas = new Canvas24(width, height);

    auto img_1 = resources.loadImageFromCompressedPath("charsprites.png");
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");
    auto img_3 = resources.loadImageFromCompressedPath("bw.png");
//
    auto *bmp_1_native = new Bitmap<coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height);
    auto *bmp_2_native = new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    auto *bmp_3_native = new Bitmap<coder::RGB888_ARRAY>(img_3.data, img_3.width, img_3.height);

    tex_1.updateBitmap(bmp_1_native->convertToBitmap<coder::RGB888_PACKED_32>());
    tex_2.updateBitmap(bmp_2_native->convertToBitmap<coder::RGB888_PACKED_32>());
    tex_mask.updateBitmap(bmp_3_native->convertToBitmap<coder::RGB888_PACKED_32>());
    resources.init();
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto ns = std::chrono::nanoseconds(1);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        render();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    return int_ms.count();
}

void loop() {
    bool quit = false;
    SDL_Event event;

    int ms = render_test(TEST_ITERATIONS);
    cout << ms << endl;

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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
