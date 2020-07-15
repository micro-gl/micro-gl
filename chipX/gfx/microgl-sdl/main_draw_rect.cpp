#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
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
//using Canvas24= Canvas<Bitmap24, CANVAS_OPT_2d_raster_USE_BIG_INT | CANVAS_OPT_2d_raster_USE_DIVISION | CANVAS_OPT_2d_raster_AVOID_RENDER_WITH_OVERFLOWS>;
//using Canvas24= Canvas<Bitmap24, CANVAS_OPT_2d_raster_FORCE_32_BIT | CANVAS_OPT_2d_raster_AVOID_RENDER_WITH_OVERFLOWS>;
using Canvas24= Canvas<Bitmap24, CANVAS_OPT_default>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;

Canvas24 * canvas;
Texture24 tex_uv;

void loop();
void init_sdl(int width, int height);
float t=0;

template <typename number>
void axial_rect() {
    //t+=0.1;//-0.01;
    canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
            tex_uv,
            t,t, 256, 256);
}

template <typename number>
void transformed_rect() {
    t+=0.005;//-0.01;
    canvas->drawRect<blendmode::Normal, porterduff::None<>, false, number>(
            tex_uv,
            matrix_3x3<number>::rotation(microgl::math::deg_to_rad(5.0f+t), 128, 128),
//            matrix_3x3<number>::identity(),
            0, 0, 256<<0, 256<<0);
//            0, 0, 12000, 12000);
}

void render() {
    canvas->clear({255,255,255,255});
//    axial_rect<float>();
    transformed_rect<float>();

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
    canvas = new Canvas24(width, height);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
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