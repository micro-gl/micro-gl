//
// Created by Tomer Shalev on 2019-06-15.
//

#include <stdio.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <FrameBuffer.h>
#include <Canvas.h>
#include <Types.h>
#include <PixelFormat.h>

typedef FrameBuffer<uint32_t> FrameBuffer32;
FrameBuffer32 * buffer = nullptr;

int main() {
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 640, 480);
//    SDL_PIXELFORMAT_RGBA8888
//    Uint32 * pixels = new Uint32[640 * 480];
    int x = 0b11111;

    color_f_t RED{1.0,0.0,0.0, 1.0};
    color_f_t YELLOW{1.0,1.0,0.0, 1.0};
    color_f_t WHITE{1.0,1.0,1.0, 1.0};
    color_f_t GREEN{0.0,1.0,0.0, 1.0};
    color_f_t BLUE{0.0,0.0,1.0, 1.0};


//    Canvas16Bit * canvas = new Canvas16Bit(640, 480, PixelFormat::RGB565);
    auto * canvas = new Canvas32Bit(640, 480, PixelFormat::RGBA8888);

    canvas->setAntialiasing(true);
//    canvas->clear(WHITE);
    canvas->drawQuad(WHITE, 0, 0, 640, 480);
    canvas->drawQuad(YELLOW, 0, 0, 320, 240);
    canvas->drawGradient(YELLOW, RED, 0, 240, 640, 140);
    canvas->setBlendMode(BlendMode::Normal);
    canvas->setPorterDuffMode(PorterDuff::SourceOver);
    canvas->drawCircle(GREEN, 320, 240, 240/2);
    canvas->drawTriangle(BLUE, 0, 300, 300, 300, 0, 0);

    SDL_UpdateTexture(texture, nullptr, canvas->pixels(), 640 * canvas->sizeofPixel());
//    SDL_UpdateTexture(texture, nullptr, buffer->pixels(), 640 * sizeof(Uint32));

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

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }


//    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}