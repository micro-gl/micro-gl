#include <iostream>
#include <chrono>
#include <SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
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
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::Bilinear>;
Texture24 tex_uv;
Canvas24 * canvas;
sampling::flat_color<> color_grey{{122,122,122,255}};
void loop();
void init_sdl(int width, int height);

template <typename number>
using cubic_mesh = vec3<number> *;

template <typename number>
vec3<number>* bi_cubic_1(){

    return new vec3<number>[4*4] {
                {1.0f, 0.0f},
                {170.66f, 0.0f},
                {341.333f, 0.0f},
                {512.0f, 0.0f},

                {1.0f,       170.66f},
                {293.44f,    162.78f},
                {746.68f,    144.65f},
                {512.0f,     170.66f},

                {1.0f,       341.33f},
                {383.12f,    327.69f},
                {1042.79f,   296.31f},
                {512.0f,     341.33f},

                {1.0f,       512.0f},
                {170.66f,    512.0f},
                {341.333f,   512.0f},
                {512.0f,     512.0f}
    };
}

template <typename number>
void render_blocks() {
    bool debug = 1;
    const auto mesh= bi_cubic_1<number>();
    int block_size = W/4;//2;//W/13;
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
            canvas->drawBezierPatch<blendmode::Normal, porterduff::None<>, false, false, number, number>(
//                    color_grey,
                    tex_uv,
                    matrix_3x3<number>::identity(),
                    mesh, 4, 4, 20, 20,
                    0, 1, 1, 0,
                    255);

            SDL_Rect rect_source {0, 0, block_size, block_size};
            SDL_Rect rect_dest {ix, iy, block_size-debug, block_size-debug};
            // very slow operation here
            SDL_UpdateTexture(sdl_texture,
                              &rect_source,
                              &canvas->pixels()[0],
                              (canvas->width()) * canvas->sizeofPixel());
            SDL_RenderCopy(renderer, sdl_texture, &rect_source, &rect_dest);
        }
    }

    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(sdl_texture);

    delete[] mesh;
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
    renderer = SDL_CreateRenderer(window, -1, 0);
    sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STREAMING, width, height);
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
//
        render();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
