#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/fast_radial_gradient.h>
#include <microgl/samplers/linear_gradient_2_colors.h>
#include <microgl/samplers/flat_color.h>
#include "src/Resources.h"

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture_sdl;

using Canvas24Bit_Packed32 = Canvas<uint32_t, coder::RGB888_PACKED_32>;
using namespace microgl;
using namespace microgl::sampling;

using Texture24= sampling::texture<uint32_t, coder::RGB888_PACKED_32, sampling::texture_sampling::NearestNeighboor>;
fast_radial_gradient<float> radial_gradient{0.5, 0.5, 0.75};
linear_gradient_2_colors<false> gradient2Colors{{255,0,255}, {255,0,0}};
linear_gradient_2_colors<true> gradient2Colors2{{0,0,255}, {0,0,0}};
flat_color flatColor{{133,133,133, 255}};
flat_color flatColorRed{{255,0,0, 255}};
Texture24 tex_1, tex_2;

Canvas24Bit_Packed32 * canvas;
Resources resources{};

void loop();
void init_sdl(int width, int height);

using namespace tessellation;

template <typename number>
void render_polygon(const dynamic_array<vec2<number>> &polygon);

float t = 0;

template <typename number>
dynamic_array<vec2<number>> poly_hole() {
    vec2<number> p0 = {100,100};
    vec2<number> p1 = {300, 100};
    vec2<number> p2 = {300, 300};
    vec2<number> p3 = {100, 300};

    vec2<number> p4 = {150,150};
    vec2<number> p7 = {150, 250};
    vec2<number> p6 = {250, 250};
    vec2<number> p5 = {250, 150};

//    return {p4, p5, p6, p7};
    return {p0, p1, p2, p3,   p4, p7, p6, p5, p4,p3};//,p5_,p4_};
}

template <typename number>
dynamic_array<vec2<number>> poly_diamond() {
    return {{300, 100}, {400, 300}, {300, 400}, {100,300}};
}

void render() {
    t+=.05f;
//    render_polygon<float>(poly_hole<float>());
    render_polygon<float>(poly_diamond<float>());
}


template <typename number>
void render_polygon(const dynamic_array<vec2<number>> &polygon) {
    using index = unsigned int;

//    polygon[1].x = 140 + 20 +  t;
//    polygon[1].y = 140 + 20 -  t;
    canvas->clear(color::colors::WHITE);

    canvas->drawPolygon<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
            gradient2Colors,
            polygon.data(),
            polygon.size(),
            255,
            polygons::hints::SIMPLE);

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
    texture_sdl = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STATIC, width, height);

    auto img_1 = resources.loadImageFromCompressedPath("charsprites.png");
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");

    auto bmp_1 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_1.data, img_1.width, img_1.height);
    auto bmp_2 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);

    tex_1.updateBitmap(bmp_1->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>());
    tex_2.updateBitmap(bmp_2->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>());

    radial_gradient.addStop(0.0f, {255,0,0});
    radial_gradient.addStop(0.45f, {255,0,0});
    radial_gradient.addStop(0.50f, {0,255,0});
    radial_gradient.addStop(1.f, {255,0,255});
    canvas = new Canvas24Bit_Packed32(width, height);
    resources.init();
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
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

        SDL_UpdateTexture(texture_sdl, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture_sdl, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
