#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/elliptic_arc_divider.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Canvas24 = Canvas<uint32_t, microgl::coder::RGB888_PACKED_32>;
Canvas24 * canvas;
sampling::flat_color color_red{{255,0,0,255}};
using uint = unsigned int;
using math = microgl::math;
using namespace microgl::tessellation;

Resources resources{};

void loop();
void init_sdl(int width, int height);

template <typename number>
void render_arc_internal(number start_angle_rad,
                         number end_angle_rad,
                         number center_x, number center_y,
                         number radius_x, number radius_y,
                         number rotation, uint divisions_count);

template <typename number>
void render_arc() {
    number start_angle_rad = math::deg_to_rad(0.0f);
    number end_angle_rad = math::deg_to_rad(180.0f);
    number rotation = math::deg_to_rad(45.0f);
    number radius_x = 50, radius_y = 75;
    number center_x = 200, center_y=200;

    render_arc_internal<number>(start_angle_rad, end_angle_rad,
            center_x, center_y, radius_x, radius_y,
            rotation,32);
}

void render() {
    render_arc<float>();
//    render_arc<Q<12>>();
}


template <typename number>
void render_arc_internal(number start_angle_rad,
                number end_angle_rad,
                number center_x, number center_y,
                number radius_x, number radius_y,
                number rotation,
                uint divisions_count) {
    using ellipse = microgl::tessellation::elliptic_arc_divider<number>;
    canvas->clear({255,255,255,255});

    dynamic_array<vec2<number>> arc_points;

    ellipse::compute(
            arc_points,
            center_x,
            center_y,
            radius_x,
            radius_y,
            rotation,
            start_angle_rad,
            end_angle_rad,
            divisions_count,
            false
            );

    for (uint ix = 0; ix < arc_points.size(); ++ix) {
        canvas->drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red, color_red,
                arc_points[ix].x,
                arc_points[ix].y,
                number(4), number(1),
                120);
    }

    canvas->drawWuLinePath(
            {0,0,0,255},
            arc_points.data(),
            arc_points.size(),
            false);

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

    canvas = new Canvas24(width, height);

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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
