//
// Created by Tomer Shalev on 2019-06-15.
// this is a sandbox for playing with microgl lib
//

#include <stdio.h>
#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/FrameBuffer.h>
#include <microgl/Canvas.h>
#include <microgl/Types.h>
#include <microgl/PixelFormat.h>
#include <microgl/PixelCoder.h>
#include <microgl/Bitmap.h>

#define TEST_ITERATIONS 1
#define W 640
#define H 480

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Bitmap<vec3<uint8_t>, RGB888_ARRAY> Bitmap24bitU8;
typedef Bitmap<uint32_t, RGB888_PACKED_32> Bitmap24bit_Packed32;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;
typedef Canvas<vec3<uint8_t >, RGB888_ARRAY> Canvas24BitU8;


//Canvas16Bit * canvas;
//Canvas24BitU8 * canvas;
Canvas24Bit_Packed32 * canvas;

Bitmap24bitU8 * bmp_1;
Bitmap24bit_Packed32 * bmp_2;

Resources resources{};
Resources::image_info_t img_1;

//Bitmap<vec3<uint8_t>> *bmp_1;

color_f_t RED{1.0,0.0,0.0, 1.0};
color_f_t YELLOW{1.0,1.0,0.0, 1.0};
color_f_t WHITE{1.0,1.0,1.0, 1.0};
color_f_t GREEN{0.0,1.0,0.0, 1.0};
color_f_t BLUE{0.0,0.0,1.0, 1.0};
color_f_t BLACK{0.0,0.0,0.0, 1.0};

void loop();
void init_sdl(int width, int height);

inline void render() {

    canvas->setAntialiasing(false);
//    canvas->clear(WHITE);

    for (int ix = 0; ix < 100; ++ix) {
        /*
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(YELLOW, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(WHITE, 0, 0, 640, 480);
        canvas->drawQuad(GREEN, 0, 0, 640, 480);
         */


        canvas->clear(YELLOW);
//        canvas->drawQuad(YELLOW, 0, 0, W/2, H/2,255);
//        canvas->drawQuad<blendmode::Normal>(GREEN, W/2, 0, W, H/2);
//        canvas->drawQuad<blendmode::Normal>(RED, W/4, 0,W/4+ W/2, H/2,128);

//        canvas->drawQuad(YELLOW, 0, 0, W, H);
//        canvas->drawGradient(YELLOW, RED, 0, 240, 640, 140);
//        canvas->drawCircle(GREEN, 320, 240, 240/2);
//        canvas->drawTriangle(RED, 0, 0, 300, 0, 300, 300);

// STARTED 420,
//        canvas->drawTriangle(*bmp_2, 0, 0, 0.0, 1.0,
//                             W, H, 1.0, 0.0,
//                             0, H, 0.0, 0.0 );
// clock-wise
//        canvas->drawTriangle<blendmode::Normal, porterduff::None>(*bmp_2, 0, 448, 0.0, 0.0,
//                                                                  252, 138, 0.0, 1.0,
//                                                                  560, 391, 1.0, 1.0 );


        // rotated cube
        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque>(*bmp_2, 0, W/2, 0.0, 0.0,
                                                                  W/2, 0, 0.0, 1.0,
                                                                  W, W/2, 1.0, 1.0 , 128);
        canvas->drawTriangle<blendmode::Normal, porterduff::None>(*bmp_2, W, W/2, 1.0, 1.0,
                                                                  W/2, W, 1.0, 0.0,
                                                                  0, W/2, 0.0, 0.0 );

// started 150, then 80 with coding (and 32 without coding)
//        canvas->drawTriangle<blendmode::Normal, porterduff::None>(YELLOW, 0, 0,
//                                     W, H,
//                                     0, H, 128);

//        canvas->drawQuad(*bmp_1, 0, 0, 640, 480);
// started 400, then 230 with re-coding, then 21 without recoding and blending
//        canvas->drawQuad<blendmode::Normal, porterduff::None>(*bmp_2, 0, 0, 640, 480, 128);

//        canvas->drawLine(BLACK, 80 , 200 , 550, 150);
//        canvas->drawLine(BLACK, 80 , 80 , 180, 400);
//        canvas->drawLine(BLACK, 80 , 80 , 600, 80);
//        canvas->drawLine(BLACK, 80 , 80 , 80, 450);

    }

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
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);

//    canvas = new Canvas16Bit(width, height, PixelFormat::RGB565, new RGB565_PACKED_16());
    canvas = new Canvas24Bit_Packed32(width, height, new RGB888_PACKED_32());
//    canvas = new Canvas24BitU8(width, height, new RGB888_ARRAY());


    img_1 = resources.loadImageFromCompressedPath("charsprites.png");


//    auto * bmp_1 = new Bitmap<uint32_t , RGB888_PACKED_32>(img_1.data, img_1.width, img_1.height, new RGB888_PACKED_32());
    bmp_1 = new Bitmap<vec3<uint8_t>, RGB888_ARRAY>(img_1.data, img_1.width, img_1.height, new RGB888_ARRAY());
    bmp_2 = bmp_1->convertToBitmap<uint32_t , RGB888_PACKED_32>();

    resources.init();
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


    cout << ms << endl;

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