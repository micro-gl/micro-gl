#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/PixelCoder.h>
#include <microgl/tesselation/arc_divider.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;
using uint = unsigned int;
using math = microgl::math;
using namespace tessellation;

Resources resources{};

void loop();
void init_sdl(int width, int height);
template <typename number>
void render_arc(number start_angle_rad,
                number end_angle_rad,
                number center_x, number center_y,
                number radius, uint divisions_count);

template <unsigned N>
void render_q_arc();
void render_float_arc();

void render() {
//    render_float_arc();
    render_q_arc<8>();
}

void render_float_arc() {
    float start_angle_rad = math::deg_to_rad(0.0f);
    float end_angle_rad = math::deg_to_rad(180.0f);
    float radius = 100;
    float center_x = 200, center_y=200;

    render_arc<float>(start_angle_rad, end_angle_rad,
                      center_x, center_y, radius, 32);
}

template <unsigned N>
void render_q_arc() {
    using q = Q<N>;
    q start_angle_rad = math::deg_to_rad(q(0));
    q end_angle_rad = math::deg_to_rad(q(360));
    q radius = 100;
    q center_x = 200, center_y=200;

    render_arc<q>(start_angle_rad, end_angle_rad,
                      center_x, center_y, radius, 8);
}

template <typename number>
void render_arc(number start_angle_rad,
                number end_angle_rad,
                number center_x, number center_y,
                number radius, uint divisions_count) {
    using arc = tessellation::arc_divider<number>;
    canvas->clear(WHITE);

    dynamic_array<vec2<number>> arc_points;

    arc::compute(
            arc_points,
            radius,
            center_x,
            center_y,
            start_angle_rad,
            end_angle_rad,
            divisions_count,
            false
            );

    for (uint ix = 0; ix < arc_points.size(); ++ix) {
        canvas->drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
                color_f_t{0.0,0.0,1.0},
                arc_points[ix].x,
                arc_points[ix].y,
                number(1),
                120);
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

void loop() {
    bool quit = false;
    SDL_Event event;

    render();

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

#pragma clang diagnostic pop