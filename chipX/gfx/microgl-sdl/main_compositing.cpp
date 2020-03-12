#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/porter_duff/SourceOver2.h>
#include <microgl/porter_duff/SourceOverOnOpaque.h>
#include <microgl/porter_duff/SourceOverOnOpaque2.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/blend_modes/Multiply.h>
#include <microgl/Bitmap.h>

#define TEST_ITERATIONS 1000
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Bitmap24= Bitmap<uint32_t, coder::RGB888_PACKED_32>;
using Canvas24= Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24 * canvas;
Bitmap24 * bmp_1, *bmp_2, *mask;

Resources resources{};
using namespace microgl::color;
void loop();
void init_sdl(int width, int height);


inline void render() {
    canvas->clear({255, 255, 255, 255});
    canvas->drawQuad<blendmode::Normal, porterduff::SourceOver2<false>>(
//    canvas->drawQuad<blendmode::Normal, porterduff::SourceOverOnOpaque>(
            {255,0,0,255},
            -0, -0, 300, 300,
            128);

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
//    canvas = new Canvas24BitU8(width, height, new RGB888_ARRAY());
    auto img_1 = resources.loadImageFromCompressedPath("charsprites.png");
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");
    auto img_3 = resources.loadImageFromCompressedPath("bw.png");
//
    auto *bmp_1_native = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height);
    auto *bmp_2_native = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    auto *bmp_3_native = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_3.data, img_3.width, img_3.height);

    bmp_1 = bmp_1_native->convertToBitmap<uint32_t, coder::RGB888_PACKED_32>();
    bmp_2 = bmp_2_native->convertToBitmap<uint32_t, coder::RGB888_PACKED_32>();
    mask = bmp_3_native->convertToBitmap<uint32_t, coder::RGB888_PACKED_32>();
    resources.init();
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto ns = std::chrono::nanoseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        render();
    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return int_ms.count();
}

void loop() {
    bool quit = false;
    SDL_Event event;

    // 100 Quads
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

//        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
