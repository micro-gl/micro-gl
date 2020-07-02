#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1
SDL_Window * sdl_window;
SDL_Renderer * sdl_renderer;
SDL_Texture * sdl_texture;
Resources resources{};
using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
//using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;

Canvas24 * canvas;
fast_radial_gradient<float> gradient{0.5, 0.5, 0.75};
linear_gradient_2_colors<120> gradient2Colors{{255,0,255}, {255,0,0}};
linear_gradient_2_colors<0> gradient2Colors2{{0,0,255}, {0,0,0}};
flat_color flatColor{{133,133,133, 255}};
flat_color flatColorRed{{255,0,0, 255}};
Texture24 tex_uv;
void loop();
void init_sdl(int width, int height);
float t=0;

template <typename number>
void test_1() {
//    t+=0.1;
    canvas->drawRoundedRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
            tex_uv,
//            gradient2Colors,
            flatColor,
            10+t, 10+t, 400+t, 400+t,
            50, 10);
}

void render() {
    canvas->clear({255, 255, 255, 255});
    test_1<float>();
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
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto bmp_uv_U8 = new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());

    gradient.addStop(0.0f, {255,0,0});
    gradient.addStop(0.45f, {255,0,0});
    gradient.addStop(0.50f, {0,255,0});
    gradient.addStop(1.f, {255,0,255});
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

        SDL_UpdateTexture(sdl_texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(sdl_renderer);
        SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
