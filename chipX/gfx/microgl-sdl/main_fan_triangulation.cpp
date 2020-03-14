#include <iostream>
#include <chrono>
#include <vector>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/fan_triangulation.h>

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

template <typename number>
void render_polygon(const dynamic_array<vec2<number>> & polygon);

float t = 0;

template <typename number>
dynamic_array<vec2<number>> poly_diamond() {
    return {
        {100,300},
        {300, 100},
        {400, 300},
        {300, 400}
    };
}

void render() {
//    t+=.05f;
//    std::cout << t << std::endl;
    render_polygon<float>(poly_diamond<float>());
}


template <typename number>
void render_polygon(const dynamic_array<vec2<number>> & polygon) {
    using index = unsigned int;
    using fan = tessellation::fan_triangulation<number>;

    canvas->clear({255,255,255,255});

    auto type = triangles::indices::TRIANGLES_FAN_WITH_BOUNDARY;

    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;

    fan::compute(
            polygon.data(),
            polygon.size(),
            indices,
            &boundary_buffer,
            type
            );

    // draw triangles batch
    canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_red,
            polygon.data(),
            (vec2<number> *)nullptr,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            120);

//    return;

    // draw triangulation
    canvas->drawTrianglesWireframe(
            {0,0,0,255},
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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
