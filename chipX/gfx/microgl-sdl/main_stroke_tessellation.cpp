#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/Q.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/stroke_tessellation.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Canvas24 = Canvas<uint32_t, microgl::coder::RGB888_PACKED_32>;
Canvas24 * canvas;
sampling::flat_color color_red{{255,0,0,255}};

void loop();
void init_sdl(int width, int height);

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

//    return {p0,p1,p2, p3, {50,50}};
    return {p0,p1,p2, p3};//, {250,320}};
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
    vec2<T> p1 = {200, 100};
    vec2<T> p2 = {200, 200};
//    vec2<T> p3 = {220, 100};
    vec2<T> p3 = {300, 200};

    return {p0, p1, p2, p3};
//    return {p0, p1};
}

void render() {
    using q = Q<4>;
    render_path(path_tri<float>(), 50.0f, false);
//    render_path(path_2<float>(), 20.0f, false);
//    render_path(path_line<float>(), 15.0f, false);
//    render_path<q>(path_2<q>(), q(10.0f), true);
//    render_path<q>(path_3<q>(), q(15.0f), false);
//    render_path<q>(path_line<q>(), q(15.0f), false);

}

template <typename number>
void render_path(const dynamic_array<vec2<number>> &path, number stroke_size, bool close_path) {
    using index = unsigned int;
    using stroke_tess = microgl::tessellation::stroke_tessellation<number>;

//    polygon[1].x = 140 + 20 +  t;
//    polygon[1].y = 140 + 20 -  t;

//    auto type = TrianglesIndices::TRIANGLES_STRIP;
    auto type = triangles::indices ::TRIANGLES_STRIP_WITH_BOUNDARY;

    dynamic_array<index> indices;
    dynamic_array<vec2<number>> vertices;
    dynamic_array<boundary_info> boundary_buffer;

    stroke_tess::compute(
            stroke_size,
            close_path,
            tessellation::stroke_cap::round,
            tessellation::stroke_line_join::bevel,
            tessellation::stroke_gravity::center,
//            tessellation::stroke_gravity::inward,
//            tessellation::stroke_gravity::outward,
            path.data(),
            path.size(),
            vertices,
            indices,
            type,
            &boundary_buffer,
            4);

    // draw triangles batch
    canvas->clear({255,255,255,255});
    canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false>(
            color_red,
            vertices.data(),
            (vec2<number> *)nullptr,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            120
            );

    return;
    // draw triangulation
    canvas->drawTrianglesWireframe(
            {0,0,0,255},
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

    canvas = new Canvas24(width, height);
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

    // 100 Quads
    int ms = render_test(TEST_ITERATIONS);
    std::cout<<ms<<std::endl;

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
