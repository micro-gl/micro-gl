//
// Created by Tomer Shalev on 2019-06-15.
// this is a sandbox for playing with microgl lib
//

#include <iostream>
#include <chrono>
//#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Bitmap<vec3<uint8_t>, RGB888_ARRAY> Bitmap24bitU8;
typedef Bitmap<uint32_t, RGB888_PACKED_32> Bitmap24bit_Packed32;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;

using bcd = tessellation::BezierCurveDivider;
//Resources resources{};


void loop();
void init_sdl(int width, int height);

vec2_32i a[3] = {{5, H - 5}, {0, 225}, {W/2, H - 5}};
vec2_32i b[7] = {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}, {W/2+W/8, H/2}, {W/2 + W/3, H/4}, {W-5, H - 5}};
vec2_32i c[5] = {{5, H - 5}, {5, 225}, {W/2, H - 5}, {W-5,255}, {W-5, H-5}};

vec2_f b_f[7] = {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}, {W/2+W/8, H/2}, {W/2 + W/3, H/4}, {W-5, H - 5}};
vec2_f b_f_small[7] = {{5, 400}, {250, 400}, {500, 400}, {150, 200}};
vec2_f c_f[5] = {{5, H - 5}, {5, 225}, {W/2, H - 5}, {W-5,255}, {W-5, H-5}};
vec2_f c2_f[3] = {{20, 400}, {20+200, 300}, {20+400, 400}};

inline void render() {

    canvas->setAntialiasing(false);

    for (int ix = 0; ix < 1; ++ix) {

        canvas->clear(WHITE);

        b_f_small[2].y -= 1.012;//0.05f;
        canvas->drawQuadraticBezierPath(BLACK, b_f_small, 3,
                bcd::CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small);

    }

}


/**
 * test cubic curve split
 */
void test_curve_adaptive_subdivide() {
//    vec2_f b_f[4] = {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}};

    vec2_32i left_1, left_2, left_3, left_4;
    vec2_32i right_1, right_2, right_3, right_4;

    vec2_32i split[7] = {left_1, left_2, left_3, left_4, right_2, right_3, right_4};

//    canvas->drawCubicBezierPath(BLACK, b_f, 4, 5);
    canvas->drawCubicBezierPath(BLACK, b_f_small, 4);
//    canvas->drawCubicBezierPath(BLACK, split, 7, 0, 4);
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

    canvas = new Canvas24Bit_Packed32(width, height, new RGB888_PACKED_32());

//    resources.init();
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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
