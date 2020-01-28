#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/PixelCoder.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/shader.h>

using namespace microgl::shading;
#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using namespace microgl::shading;
using index_t = unsigned int;
using Canvas24Bit_Packed32 = Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24Bit_Packed32 * canvas;

void loop();
void init_sdl(int width, int height);

float t = 0;

void test_shader_1() {
    color_shader shader;
    canvas->clear(color::colors::WHITE);
    simple_vertex_attributes<float> v0, v1, v2;
    v0.point= {10.0,10.0, 0};  v0.color= {255,0,0,255};
    v1.point= {400.0,200.0, 0}; v1.color= {0,255,0,255};
    v2.point= {10.0,400.0, 0}; v2.color= {0,0,255,255};
    canvas->drawTriangleShader(shader, v0, v1, v2,255);
//    canvas->drawTriangle(color::colors::RED, 10.0,10.0, 400.0,10.0, 400.0,400.0, 255);
}

void render() {
    using index = unsigned int;

    canvas->clear(color::colors::WHITE);

    test_shader_1();
//        canvas->drawQuad(RED, 0, 0, 100,100, 0,255);
//    canvas->drawPolygon<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//            chunk.data,
//            chunk.size,
//            120,
//            polygons::hints::SIMPLE
//    );


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
