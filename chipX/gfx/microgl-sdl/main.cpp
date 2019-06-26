//
// Created by Tomer Shalev on 2019-06-15.
// this is a sandbox for playing with microgl lib
//

#include <stdio.h>
#include <iostream>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/FrameBuffer.h>
#include <microgl/Canvas.h>
#include <microgl/Types.h>
#include <microgl/PixelFormat.h>
#include <microgl/PixelCoder.h>
#include <microgl/Bitmap.h>

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
//Canvas16Bit * canvas;
Canvas24BitU8 * canvas;
//Canvas32Bit * canvas;
Resources resources{};

color_f_t RED{1.0,0.0,0.0, 1.0};
color_f_t YELLOW{1.0,1.0,0.0, 1.0};
color_f_t WHITE{1.0,1.0,1.0, 1.0};
color_f_t GREEN{0.0,1.0,0.0, 1.0};
color_f_t BLUE{0.0,0.0,1.0, 1.0};

void loop();
void init_sdl(int width, int height);

inline void render() {

//    Resources::image_info_t img_1 = resources.loadImageFromCompressedPath("a.png");
    Resources::image_info_t img_1 = resources.loadImageFromCompressedPath("charsprites.png");

    Bitmap<vec3<uint8_t>> bmp(img_1.data, img_1.width, img_1.height, new RGB888_ARRAY());
//    Bitmap<vec4<uint8_t>> bmp(img_1.data, img_1.width, img_1.height, new RGBA8888_ARRAY());
//    Bitmap<uint32_t > bmp(img_1.data, img_1.width, img_1.height, new RGBA8888_PACKED_32());

    canvas->setAntialiasing(true);
//    canvas->clear(WHITE);
    canvas->drawQuad(WHITE, 0, 0, 640, 480);
    canvas->drawQuad(YELLOW, 0, 0, 320, 240);
    canvas->drawGradient(YELLOW, RED, 0, 240, 640, 140);
    canvas->setBlendMode(BlendMode::Normal);
    canvas->setPorterDuffMode(PorterDuff::SourceOver);
    canvas->drawCircle(GREEN, 320, 240, 240/2);

//    canvas->drawTriangle(BLUE, 0, 300, 300, 300, 0, 0);
//    canvas->drawTriangle(img_1.data, img_1.width, img_1.height,0, 300,0.0,0.0, 300, 300,1.0,0.0, 0, 0,0.0,1.0);
//    canvas->drawTriangle2(bmp,0, 300,0.0,0.0, 300, 300,1.0,0.0, 0, 0,0.0,1.0);
    canvas->drawQuad2(bmp, 300, 50, 300, 300);
}

int main() {
    init_sdl(640, 480);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);


//    canvas = new Canvas16Bit(width, height, PixelFormat::RGB565, new RGB565_PACKED_16());
    canvas = new Canvas24BitU8(width, height, new RGB888_ARRAY());
//    canvas = new Canvas32Bit(width, height, PixelFormat::RGBA8888, new RGBA8888_PACKED());



    // 24 bit with given bitmap canvas
//    auto * bmp = new Bitmap24bitU8(width, height, new RGB888_ARRAY());
//    canvas = new Canvas24BitU8(bmp);

    resources.init();
}

void loop() {
    bool quit = false;
    SDL_Event event;

    render();

    while (!quit)
    {
        SDL_WaitEvent(&event);

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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }


//    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_Quit();
}