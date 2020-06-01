#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/monotone_polygon_triangulation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Canvas<uint32_t, coder::RGB888_PACKED_32> Canvas24Bit_Packed32;
sampling::flat_color color_red{{255,0,0,255}};
sampling::flat_color color_black{{0,0,0,255}};
Canvas24Bit_Packed32 * canvas;

Resources resources{};

void loop();
void init_sdl(int width, int height);

using namespace tessellation;

template <typename number>
void render_polygon(dynamic_array<vec2<number>> polygon);

float t = 0;

template <typename number>
dynamic_array<vec2<number>> poly_rect() {
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p1 = {300, 100};
    vertex p2 = {300, 300};
    vertex p3 = {100, 300};
    return {p0, p1, p2, p3};
}

template <typename number>
dynamic_array<vec2<number>> poly_tri() {
    using vertex=vec2<number>;
    vertex p0 = {100,100};
    vertex p3 = {300, 100};
    vertex p4 = {100, 300};

    return {p0, p3, p4};
}

template <typename number>
dynamic_array<vec2<number>> poly_1() {
    return {
            {50,100},
            {100,50},
            {150,100},
            {200,50},
            {300,100},
            {400,50},
            {500,100},

            {500,200},
            {400,100+50},
            {300,100+100},
            {200,100+50},
            {150,100+100},
            {100,100+50},
            {50,100+100},

    };
}

template <typename number>
dynamic_array<vec2<number>> poly_2() {
//    return {
//            {50,200},
//            {50,100},
//            {100,150},
//            {150,100},
//            {150,200},
////            {200,150},
////            {250,100},
////            {300,150},
////            {350,100},
////            {400,150},
////            {400,200},
//
//    };
    return {
            {50,200},
            {50,100},
            {100,150},
            {150,100},
            {200,190},
            {250,100},
            {300,150},
            {350,100},
            {400,150},
            {400,200},

    };
}

void render() {
    t+=.05f;
//    std::cout << t << std::endl;
//    render_polygon(poly_rect());
    render_polygon<float>(poly_2<float>());
//    render_polygon<float>(poly_1<float>());
//    render_polygon<float>(poly_hole<float>());
//    render_polygon<float>(poly_hole3<float>());
//    render_polygon<float>(poly_hole4<float>());
//    render_polygon<float>(poly_rect<float>());
//    render_polygon<float>(poly_tri<float>());
}


template <typename number>
void render_polygon(dynamic_array<vec2<number>> polygon) {
    using index = unsigned int;

//    polygon[1].x = 140 + 20 +  t;

    canvas->clear({255,255,255,255});

    using mpt = tessellation::monotone_polygon_triangulation<number>;

    triangles::indices type;
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;

    mpt::compute(polygon.data(),
                 polygon.size(),
                 mpt::monotone_axis::x_monotone,
                 indices,
                 &boundary_buffer,
                 type
    );

    // draw triangles batch
    canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_red,
            matrix_3x3<number>::identity(),
            polygon.data(),
            (vec2<number> *)nullptr,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            122);

//    return;

    // draw triangulation
    canvas->drawTrianglesWireframe(
            {0,0,0,255},
            matrix_3x3<number>::identity(),
            polygon.data(),
            indices.data(),
            indices.size(),
            type,
            255);
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

    canvas = new Canvas24Bit_Packed32(width, height);

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