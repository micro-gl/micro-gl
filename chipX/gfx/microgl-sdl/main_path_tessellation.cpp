#include <iostream>
#include <chrono>
#include <vector>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/Q.h>
#include <microgl/PixelCoder.h>
#include <microgl/tesselation/path_tessellation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;

void loop();
void init_sdl(int width, int height);

using namespace tessellation;

template <typename T>
void render_path(const dynamic_array<vec2<T>> &path, T stroke_size, bool close_path);

float t = 0;
int M = 5;

template <typename T>
dynamic_array<vec2<T>> path_diagonal() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {200, 200};
    vec2<T> p2 = {300, 300};

    return {p0, p1, p2};
}

template <typename T>
dynamic_array<vec2<T>> path_horizontal() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {500, 100};
    return {p0, p1};
//    return {p0<<M, p1<<M, p2<<M};
}

template <typename T>
dynamic_array<vec2<T>> path_resh() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {300, 300};
//    return {p0<<M, p1<<M};
    return {p0, p1, p2};
}

template <typename T>
dynamic_array<vec2<T>> path_2() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 400};
    vec2<T> p2 = {400, 100};
    vec2<T> p3 = {400, 300};

    return {p0,p1,p2, p3};
    return {p0, p1, p2, p3};
}

template <typename T>
dynamic_array<vec2<T>> path_3() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {300, 300};
    vec2<T> p3 = {400, 300};

    return {p0, p1,p2};//, p3};
}


template <typename T>
dynamic_array<vec2<T>> path_line() {
    vec2<T> p0 = {100,100};
    vec2<T> p1 = {300, 100};

    return {p0, p1};
}

template <typename T>
dynamic_array<vec2<T>> path_tri() {
    vec2<T> p0 = {100, 100};
    vec2<T> p1 = {300, 100};
    vec2<T> p2 = {200, 300};

    return {p0, p1, p2};
}

void render() {
    using q = Q<4>;
//    render_path(path_2<float>(), 20.0f, true);
//    render_path(path_line<float>(), 15.0f, false);
    render_path<q>(path_2<q>(), q(10.0f), true);
//    render_path<q>(path_3<q>(), q(15.0f), false);
//    render_path<q>(path_line<q>(), q(15.0f), false);

}

template <typename T>
void render_path(const dynamic_array<vec2<T>> &path, T stroke_size, bool close_path) {
    using index = unsigned int;
    using tri = triangles::TrianglesIndices;
    using path_tess = tessellation::path_tessellation<T>;

//    polygon[1].x = 140 + 20 +  t;
//    polygon[1].y = 140 + 20 -  t;

//    auto type = TrianglesIndices::TRIANGLES_STRIP;
    auto type = TrianglesIndices::TRIANGLES_STRIP_WITH_BOUNDARY;

    dynamic_array<index> indices;
    dynamic_array<vec2<T>> vertices;
    dynamic_array<boundary_info> boundary_buffer;

    path_tess::compute(
            stroke_size,
            close_path,
            tessellation::path_gravity::center,
//            tessellation::path_gravity::inward,
//            tessellation::path_gravity::outward,
            path.data(),
            path.size(),
//            precision,
            indices,
            vertices,
            &boundary_buffer,
            type
    );

    // draw triangles batch
    canvas->clear(WHITE);
    canvas->drawTriangles<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
            RED,
            vertices.data(),
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            120
            );

    return;
    // draw triangulation
    canvas->drawTrianglesWireframe(BLACK,
            vertices.data(),
            indices.data(),
            indices.size(),
            type,
            255
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
//        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
