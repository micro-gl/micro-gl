#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/shaders/color_shader.h>
#include <microgl/shaders/flat_color_shader.h>
#include <microgl/shaders/sampler_shader.h>
#include <microgl/samplers/texture.h>
#include <microgl/camera.h>

using namespace microgl::shading;
#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
Resources resources{};

using namespace microgl::shading;
using index_t = unsigned int;

using Bitmap24= Bitmap<uint32_t, coder::RGB888_PACKED_32>;
using Canvas24= Canvas<uint32_t, coder::RGB888_PACKED_32>;
using Texture24= sampling::texture<uint32_t, coder::RGB888_PACKED_32, sampling::texture_sampling::NearestNeighboor>;
Canvas24 * canvas;
Texture24 tex_1, tex_2;

void loop();
void init_sdl(int width, int height);

float t = 0;

// bitmap for mapping
Bitmap<uint32_t, coder::RGB888_PACKED_32> *bmp_uv;

template <typename number>
void test_shader_color_2d() {
    color_shader<number> shader;
    shader.mat= camera<number>::orthographic(0, W, 0, H, 1, 100);

    color_shader_vertex_attributes<number> v0, v1, v2;
    v0.point= {10.0,10.0, 0};  v0.color= {255,0,0,255};
    v1.point= {400.0,200.0, 0}; v1.color= {0,255,0,255};
    v2.point= {10.0,400.0, 0}; v2.color= {0,0,255,255};

    canvas->drawTriangle<blendmode::Normal, porterduff::None, true>(shader, v0, v1, v2, 255);
//    canvas->drawTriangle(color::colors::RED, 10.0,10.0, 400.0,10.0, 400.0,400.0, 255);
}

template <typename number>
void test_shader_flat_color_2d() {
    flat_color_shader<number> shader;
    color_t color;
    canvas->coder().convert(color::colors::RED, color);
    shader.matrix= camera<number>::orthographic(0, W, 0, H, 1, 100);
    shader.color= color;
    flat_color_shader_vertex_attributes<number> v0, v1, v2;
    v0.point= {10.0,10.0, 0};
    v1.point= {500.0,10.0, 0};
    v2.point= {500.0,500.0, 0};

    canvas->drawTriangle<blendmode::Normal, porterduff::None, false>(shader, v0, v1, v2, 255);
    //canvas->drawTriangle<blendmode::Normal, porterduff::None, false>(shader, v0, v1, v2, 255);
//    canvas->drawTriangle<blendmode::Normal, porterduff::None, false>(color::colors::RED,
//            10.0,10.0, 400.0,200.0, 10.0,400.0, 255);
}

template <typename number>
void test_shader_texture_2d() {

    sampler_shader<number, Texture24> shader;
    shader.matrix= camera<number>::orthographic(0, W, 0, H, 0, 10);
    shader.sampler= &tex_1;

    sampler_shader_vertex_attribute<number> v0{}, v1{}, v2{};
    v0.point= {10.0,10.0, 0};   v0.uv= {0.0f, 0.0f};
    v1.point= {500.0,10.0, 0};  v1.uv= {1.0f, 0.0f};
    v2.point= {500.0,500.0, 0}; v2.uv= {1.0f, 1.0f};

    canvas->drawTriangle<blendmode::Normal, porterduff::None, false>(shader, v0, v1, v2, 255);
    return;
    canvas->drawTriangle<blendmode::Normal, porterduff::None>(tex_1,
            10.0,10.0, 0.0, 0.0,
            500.0,10.0, 1.0, 0.0,
            500.0,500.0, 1.0, 1.0,
            255);
}

void render() {
    canvas->clear(color::colors::WHITE);

//    test_shader_color_2d<float>();
//    test_shader_color_2d<Q<10>>();
    test_shader_texture_2d<Q<10>>();
//    test_shader_texture_2d<float>();
//    test_shader_flat_color_2d<float>();
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
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");
    auto bmp_uv_U8 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_1.updateBitmap(bmp_uv_U8->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>());
    canvas = new Canvas24(width, height);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        render();
    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return int_ms.count();}

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
