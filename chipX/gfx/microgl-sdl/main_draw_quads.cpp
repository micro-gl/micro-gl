#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/Bilinear.h>
#include <microgl/samplers/NearestNeighbor.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Multiply.h>
#include <microgl/blend_modes/Difference.h>
#include <microgl/Q.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY> Bitmap24bitU8;
typedef Bitmap<uint32_t, coder::RGB888_PACKED_32> Bitmap24bit_Packed32;

typedef Canvas<uint32_t, coder::RGB888_PACKED_32> Canvas24Bit_Packed32;
typedef Canvas<vec3<uint8_t >, coder::RGB888_ARRAY> Canvas24BitU8;

Canvas24Bit_Packed32 * canvas;

Bitmap24bitU8 * bmp_1;
Bitmap24bit_Packed32 * bmp_2;
Bitmap24bitU8 * bmp_uv_U8;
Bitmap24bit_Packed32 * bmp_uv;

Resources resources{};
Resources::image_info_t img_1;

void loop();
void init_sdl(int width, int height);

void render_float_quads() {
    static float d =0;
    float G = 256;
    d+=1.01;
//    canvas->drawQuad<blendmode::Normal, porterduff::None, sampler::NearestNeighbor>(
//            *bmp_uv,
//            0.0, 0.0, 400.0, 400.0);
//
    canvas->drawQuad<blendmode::Normal, porterduff::None>(
            color::colors::RED,
            0.0, 0.0, 300.0, 300.0,
            255);

}

void render_Q_quads() {
    static float d =0;
    float G = 256;
    d +=(1.0f);

    canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, true, sampler::Bilinear, Q<9>>(
            *bmp_uv,
            0.0f,               0.0f,     0.0f, 1.0f,
            G + 100.0f + d,     0.0f,       1.0f, 1.0f,
            G + 0.0f,                G,         1.0f, 0.0f,
            0.0f,                    G,         0.0f, 0.0f,
            255);

}

inline void render() {

    canvas->setAntialiasing(false);

    for (int ix = 0; ix < 1; ++ix) {

        canvas->clear(color::colors::WHITE);

        render_float_quads();
//        render_Q_quads();
    }

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
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);

//    canvas = new Canvas16Bit(width, height, PixelFormat::RGB565, new RGB565_PACKED_16());
    canvas = new Canvas24Bit_Packed32(width, height);
//    canvas = new Canvas24BitU8(width, height, new RGB888_ARRAY());

    img_1 = resources.loadImageFromCompressedPath("charsprites.png");
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");

//    auto * bmp_1 = new Bitmap<uint32_t , RGB888_PACKED_32>(img_1.data, img_1.width, img_1.height, new RGB888_PACKED_32());
    bmp_1 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height);
    bmp_2 = bmp_1->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>();

    bmp_uv_U8 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    bmp_uv = bmp_uv_U8->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>();
//    bmp_uv = new Bitmap<uint32_t , RGB888_PACKED_32>(img_2.width, img_2.height, new RGB888_PACKED_32());

    resources.init();
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        render();
    }

    auto end = std::chrono::high_resolution_clock::now();

    return (end-start)/(ms*N);
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
