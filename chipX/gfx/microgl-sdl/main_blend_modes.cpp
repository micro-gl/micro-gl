#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/porter_duff/FastSourceOverOnOpaque.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/blend_modes/Multiply.h>
#include <microgl/blend_modes/Overlay.h>
#include <microgl/blend_modes/PinLight.h>
#include <microgl/blend_modes/Screen.h>
#include <microgl/blend_modes/Subtract.h>
#include <microgl/blend_modes/Darken.h>
#include <microgl/blend_modes/Lighten.h>
#include <microgl/blend_modes/Difference.h>
#include <microgl/blend_modes/Divide.h>
#include <microgl/blend_modes/ColorBurn.h>
#include <microgl/blend_modes/ColorDodge.h>
#include <microgl/blend_modes/Exclusion.h>
#include <microgl/blend_modes/HardLight.h>
#include <microgl/blend_modes/VividLight.h>
#include <microgl/blend_modes/HardMix.h>
#include <microgl/blend_modes/LinearBurn.h>
#include <microgl/blend_modes/LinearDodge.h>
#include <microgl/blend_modes/LinearLight.h>
#include <microgl/blend_modes/SoftLight.h>
#include <microgl/Bitmap.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1000
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
sampling::flat_color color_grey{{122,122,122,255}};

Canvas24 * canvas;
Bitmap24 * bmp_1, *bmp_2, *mask;

Resources resources{};
using namespace microgl::color;
void loop();
void init_sdl(int width, int height);


inline void render() {
    canvas->clear({128, 255, 255, 255});
    canvas->drawQuad<blendmode::Multiply<false>, porterduff::FastSourceOverOnOpaque>(
//    canvas->drawQuad<blendmode::Normal, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Overlay<false>, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::PinLight, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Screen<true>, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Subtract, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Darken, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Lighten, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Difference, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Divide, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::ColorDodge, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::Exclusion<true>, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::HardLight<false, false>, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::VividLight, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::HardMix, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::LinearLight, porterduff::None<true>>(
//    canvas->drawQuad<blendmode::SoftLight<true>, porterduff::None<true>>(
            color_grey,
            -0, -0, 300, 300,
            255);

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

//        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
