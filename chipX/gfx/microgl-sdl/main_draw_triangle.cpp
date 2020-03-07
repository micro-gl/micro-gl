#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1
SDL_Window * sdl_window;
SDL_Renderer * sdl_renderer;
SDL_Texture * sdl_texture;

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
using Bitmap24= Bitmap<uint32_t, coder::RGB888_PACKED_32>;
using Canvas24= Canvas<uint32_t, coder::RGB888_PACKED_32>;
using Texture24= sampling::texture<uint32_t, coder::RGB888_PACKED_32, sampling::texture_sampling::NearestNeighboor>;

Resources resources{};
Canvas24 * canvas;
Texture24 tex_1, tex_2;
flat_color color_sampler{{255,122,0}};

void loop();
void init_sdl(int width, int height);
float t=0;

template <typename number>
void test_1() {
    t+=0.001;
    canvas->drawTriangle<blendmode::Normal, porterduff::None>(
            color_sampler,
            10.0f, 10.0f, 0.0f, 0.0f,
            400.0f, 10.0f, 1.0f, 0.0f,
            400.0f, 400.0f, 1.0f, 1.0f,
            255);

    return;
    canvas->drawTriangle<blendmode::Normal, porterduff::None>(
            color::colors::RED,
            10.0f, 10.0f,
            400.0f, 10.0f,
            400.0f, 400.0f,
            255);

//    canvas->drawQuad<blendmode::Normal, porterduff::None>(color::colors::RED, 0, 0, 400, 400);
}

template <typename number>
void test_2() {
//    t+=0.01;

    canvas->drawTriangle<blendmode::Normal, porterduff::None>(tex_1,
                                                              10.0+t,10.0, 0.0, 0.0,
                                                              500.0+0,10.0, 1.0, 0.0,
                                                              500.0+0,500.0, 1.0, 1.0,
                                                              255);
}

template <typename number>
void test_3_aa() {
//    t+=0.01;

    canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(tex_1,
                                                              10.0,10.0, 0.0, 1.0,
                                                              500.0+t,10.0, 1.0, 1.0,
                                                              500.0+t,500.0, 1.0, 0.0,
                                                              255);
}



void render() {
    canvas->clear(color::colors::WHITE);

//    test_1<float>();
//    test_2<float>();
    test_3_aa<float>();

}

int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    sdl_window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888,
                                    SDL_TEXTUREACCESS_STATIC, width, height);
    auto img_2 = resources.loadImageFromCompressedPath("uv_512.png");
    auto bmp_uv_U8 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_1.updateBitmap(bmp_uv_U8->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>());
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
//
        render();

        SDL_UpdateTexture(sdl_texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(sdl_renderer);
        SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
