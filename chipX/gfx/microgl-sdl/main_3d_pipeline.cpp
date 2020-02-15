#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/color.h>
#include <microgl/Canvas.h>
#include <microgl/matrix_4x4.h>
#include <microgl/camera.h>
#include <microgl/pipeline.h>
#include <microgl/dynamic_array.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include "data/model_3d_tree.h"
#include "data/model_3d_cube.h"

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
using Canvas24Bit_Packed32 = Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24Bit_Packed32 * canvas;

float t = 0.0f;

void loop();
void init_sdl(int width, int height);

template <typename number>
using arr = dynamic_array<vec3<number>>;

float z = 0;

template <typename number_coords>
void render_template(const model_3d<number_coords> & object) {
    using vertex = vec3<number_coords>;
    using camera = microgl::camera<number_coords>;
    using mat4 = matrix_4x4<number_coords>;
    using math = microgl::math;

    z+=0.1121;

    int canvas_width = canvas->width();
    int canvas_height = canvas->height();

    mat4 model = mat4::transform({ 0, math::deg_to_rad(z), math::deg_to_rad(0/2)},
                                 {0,0,-300+z}, {10,10,10});
//    mat4 view = camera::lookAt({0, 0, -z}, {0,0, -z-1}, {0,1,0});
    mat4 view = camera::lookAt({0, 0, 100}, {0,0, 0}, {0,1,0});
    mat4 projection = camera::perspective(math::deg_to_rad(60),
                                          canvas_width, canvas_height, 1, 500);
//    mat4 projection = camera::perspective(-1,1,-1,1,1,10000);
    mat4 mvp = projection * view * model;
    canvas->clear(color::colors::WHITE);
    microgl::_3d::pipeline<number_coords, decltype(*canvas)>::render (
            object.vertices.data(),
            object.vertices.size(),
            object.indices.data(),
            object.indices.size(),
            mvp,
            triangles::indices::TRIANGLES,
            *canvas
            );

}

void render() {

    render_template<float>(tree_3d<float>);
//    render_template<float>(cube_3d<float>);
//    render_template<Q<10>>(tree_3d<Q<10>>);
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
    canvas = new Canvas24Bit_Packed32(width, height);
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
//    int ms = render_test(TEST_ITERATIONS);
//    std::cout << ms << std::endl;

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

        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
