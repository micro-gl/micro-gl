#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/PixelCoder.h>
#include <microgl/tesselation/PathTessellation.h>

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
void render_path(std::vector<vec2<T>> path);

float t = 0;
int M = 4;

std::vector<vec2_32i> path_diagonal() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {200, 200};
    vec2_32i p2 = {300, 300};

    return {p0, p1, p2};
}

std::vector<vec2_32i> path_horizontal() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {500, 100};
    return {p0, p1, p2};
}

std::vector<vec2_32i> path_resh() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {300, 300};
//    return {p0<<M, p1<<M};
    return {p0<<M, p1<<M, p2<<M};
}
std::vector<vec2_32i> path_2() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {400, 200};
    vec2_32i p3 = {400, 300};

//    return {p2<<M, p3<<M};
    return {p0<<M, p1<<M, p2<<M, p3<<M};
}

void render() {
//    t+=.05f;
//    std::cout << t << std::endl;
    render_path(path_2());
}


template <typename T>
void render_path(std::vector<vec2<T>> path) {
    using index = unsigned int;
    using tri = triangles::TrianglesIndices;

//    polygon[1].x = 140 + 20 +  t;
//    polygon[1].y = 140 + 20 -  t;
    canvas->clear(WHITE);

    PathTessellation path_tess{true};

    uint8_t precision = M;
    index stroke = 20<<precision;
    auto type = TrianglesIndices::TRIANGLES_STRIP;
//    index indices[size_indices];
    static_array<index, 128> indices;
    static_array<vec2_32i, 128> vertices;

    path_tess.compute(stroke,
            path.data(),
            path.size(),
            precision,
            indices,
            vertices,
            type,
            false
            );

//    indices.pop_back();
    // draw triangles batch
    canvas->drawTriangles<blendmode::Normal, porterduff::SourceOverOnOpaque, false>(
            RED,
            vertices.data(),
            indices.data(),
            indices.size(),
            type,
            120,
            precision);

//    return;
    // draw triangulation
    canvas->drawTrianglesWireframe(BLACK,
            vertices.data(),
            indices.data(),
            indices.size(),
            type,
            255,
            precision);

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