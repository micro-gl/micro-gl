#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/PixelCoder.h>
#include <microgl/tesselation/ear_clipping_triangulation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;

Resources resources{};

void loop();
void init_sdl(int width, int height);

using namespace tessellation;

template <typename T>
void render_polygon(std::vector<vec2<T>> polygon);

float t = 0;

std::vector<vec2_32i> poly_rect() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {300, 300};
    vec2_32i p3 = {100, 300};

    return {p0, p1, p2, p3};
}

float b = 1;
std::vector<vec2_f> poly_2() {
    vec2_f p0 = {100/b,100/b};
    vec2_f p1 = {300/b, 100/b};
    vec2_f p2 = {300/b, 300/b};
    vec2_f p3 = {200/b, 200/b};
    vec2_f p4 = {100/b, 300/b};

    return {p0, p1, p2, p3, p4};
}

std::vector<vec2_f> poly_tri() {
    vec2_f p0 = {100,100};
    vec2_f p3 = {300, 100};
    vec2_f p4 = {100, 300};

    return {p0, p3, p4};
}

std::vector<vec2_32i> poly_hole() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {300, 300};
    vec2_32i p3 = {100, 300};

    vec2_32i p0_1 = {150,150};
    vec2_32i p1_1 = {150, 200};
    vec2_32i p2_1 = {200, 200};
    vec2_32i p3_1 = {200, 150};

    vec2_32i p0_2 = {200,200};
    vec2_32i p1_2 = {200, 300};
    vec2_32i p2_2 = {300, 300};
    vec2_32i p3_2 = {300, 200};

//    return {p4, p5, p6, p7};
    return {p0, p1, p2, p3,
            p0_1, p1_1, p2_1,
            p0_2, p1_2, p2_2, p3_2,
            p0_2,
            p3_1,
            p0_1,p3};
}

std::vector<vec2_32i> poly_hole2() {
    return {
            {10,10},
            {400,10},
            {400,400},
            {10,400},

            {10,10},

            {10,10},
            {10,400},
            {400,400},
            {400,10},

            {10,10},
    };
}

std::vector<vec2_f> poly_hole3() {
    return {
            {10,10},
            {400,10},
            {400,400},
            {10,400},

            {10,10},

            {20,20},
            {20,400-20},
            {400-20,400-20},
            {400-20,20},

            {20,20},
    };
}

void render() {
    t+=.05f;
//    std::cout << t << std::endl;
//    render_polygon(poly_rect());
    render_polygon(poly_2());
//    render_polygon(poly_hole3());
//    render_polygon(poly_tri());
}


template <typename T>
void render_polygon(std::vector<vec2<T>> polygon) {
    using index = unsigned int;

//    polygon[1].x = 140 + 20 +  t;

    canvas->clear(WHITE);

    using ear = tessellation::ear_clipping_triangulation<T>;

    auto type = TrianglesIndices::TRIANGLES_WITH_BOUNDARY;
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;

    ear::compute(polygon.data(),
            polygon.size(),
            indices,
            &boundary_buffer,
            type
            );

    // draw triangles batch
    canvas->drawTriangles<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
            RED,
            matrix_3x3<T>::identity(),
            polygon.data(),
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            122);

//    return;

    // draw triangulation
    canvas->drawTrianglesWireframe(
            BLACK,
            matrix_3x3<T>::identity(),
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
