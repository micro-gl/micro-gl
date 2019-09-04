#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/PixelCoder.h>
#include <microgl/tesselation/ArcDivider.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;
using uint = unsigned int;

Resources resources{};

void loop();
void init_sdl(int width, int height);

using namespace tessellation;

uint deg_to_rad(float degrees, uint requested_precision) {
    return ((degrees*PI)/180.0f)*(1u<<requested_precision);
}

void render() {
    canvas->clear(WHITE);

    ArcDivider arc_divider{};
    precision precision = 5;
    uint radius = 50u<<precision;
    int start_angle = deg_to_rad(0.0f, precision);
    int end_angle = deg_to_rad(180.0f, precision);
    static_array<vec2_32i, 128> arc_points;

    arc_divider.compute(
            radius,
            start_angle,
            end_angle,
            precision,
            5,
            true,
            arc_points
            );

    for (uint ix = 0; ix < arc_points.size(); ++ix) {
        canvas->drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
                color_f_t{0.0,0.0,1.0},
                arc_points[ix].x,
                arc_points[ix].y,
                5<<precision,
                precision,
                255);
    }

    canvas->drawLinePath(
            BLACK,
            arc_points.data(),
            arc_points.size(),
            false
            );

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
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STATIC, width, height);

    canvas = new Canvas24Bit_Packed32(width, height, new RGB888_PACKED_32());

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
//
        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

#pragma clang diagnostic pop