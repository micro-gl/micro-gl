#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/Q.h>

#define TEST_ITERATIONS 10
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Bitmap24= Bitmap<uint32_t, coder::RGB888_PACKED_32>;
using Canvas24= Canvas<uint32_t, coder::RGB888_PACKED_32>;
using Texture24= sampling::texture<uint32_t, coder::RGB888_PACKED_32, sampling::texture_sampling::Bilinear>;
Canvas24 * canvas;
Texture24 tex_1, tex_2;

Resources resources{};

void loop();
void init_sdl(int width, int height);

template <typename number>
void render_quadrilateral() {
    static float d =0;
    float G = 256;
    d+=1.01;
    canvas->clear(intensity<number>{1, 1, 1, 1});
    canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number>(
            tex_2,
            0.0f,               0.0f,     0.0f, 1.0f,
            G + 100.0f + d,     0.0f,       1.0f, 1.0f,
            G + 0.0f,                G,         1.0f, 0.0f,
            0.0f,                    G,         0.0f, 0.0f,
            255);

}

inline void render() {
    render_quadrilateral<float>();
//    render_quadrilateral<Q<15>>();
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
//    auto img_2 = resources.loadImageFromCompressedPath("uv_512.png");
    auto img_2 = resources.loadImageFromCompressedPath("uv_512.png");

    auto bmp_1 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height);
    auto bmp_2 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);

    tex_1.updateBitmap(bmp_1->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>());
    tex_2.updateBitmap(bmp_2->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>());
    resources.init();
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto ns = std::chrono::nanoseconds(1);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) render();

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

        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
