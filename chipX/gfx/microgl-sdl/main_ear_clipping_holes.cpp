#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/dynamic_array.h>
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

void loop();
void init_sdl(int width, int height);

using namespace tessellation;
using namespace microgl;

dynamic_array<vec2_f> box(float left, float top,
                          float right, float bottom,
                          bool ccw=false) {
    if(!ccw)
        return {
                {left,top},
                {right,top},
                {right,bottom},
                {left,bottom},
        };

    return{
            {left,top},
            {left,bottom},
            {right,bottom},
            {right,top},
    };
};

void test_1() {
    using index = unsigned int;
    using ect = ear_clipping_triangulation;
    uint8_t precision = 0;
    auto type = TrianglesIndices::TRIANGLES_WITH_BOUNDARY;
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;
    dynamic_array<ect::hole> holes;

    dynamic_array<vec2_f> outer = box(10,10,400,400);
    dynamic_array<vec2_f> inner_1 = box(20,20,100,100, true);

    ect::hole hole_1;
    hole_1.points = inner_1.data();
    hole_1.size = inner_1.size();

    holes.push_back(hole_1);

    ect::compute(
            outer.data(),
            outer.size(),
            indices,
            type,
            &boundary_buffer,
            &holes,
            nullptr
    );

    canvas->clear(WHITE);
    // draw triangles batch
    canvas->drawTriangles<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
            RED,
            outer.data(),
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            122,
            precision);

    return;

    // draw triangulation
    canvas->drawTrianglesWireframe(
            BLACK,
            outer.data(),
            indices.data(),
            indices.size(),
            type,
            255,
            precision);

}

void render() {
    test_1();
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
