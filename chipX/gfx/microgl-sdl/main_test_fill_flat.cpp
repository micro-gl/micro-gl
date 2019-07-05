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
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

//typedef Bitmap<vec3<uint8_t>, CODER> Bitmap24bitU8<CODER>;
typedef Bitmap<uint32_t, RGB888_PACKED_32> Bitmap32bitPacked;
typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas32Bit;
//typedef Bitmap<uint16_t, CODER> Bitmap16BitPacked;
//typedef Bitmap<uint8_t, CODER> Bitmap8Bit;


//Canvas16Bit * canvas;
//Canvas24BitU8 * canvas;
Canvas32Bit * canvas;
Resources resources{};
Resources::image_info_t img_1;

//Bitmap<vec3<uint8_t>> *bmp_1;

color_f_t RED{1.0,0.0,0.0, 1.0};
color_f_t YELLOW{1.0,1.0,0.0, 1.0};
color_f_t WHITE{1.0,1.0,1.0, 1.0};
color_f_t GREEN{0.0,1.0,0.0, 1.0};
color_f_t BLUE{0.0,0.0,1.0, 1.0};

void loop();
void init_sdl(int width, int height);

inline void render() {

    canvas->setAntialiasing(false);
//    canvas->clear(WHITE);

    for (int ix = 0; ix < 100; ++ix) {
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

    }

//    canvas->drawQuad(YELLOW, 0, 0, 320, 240);
//    canvas->drawGradient(YELLOW, RED, 0, 240, 640, 140);
//    canvas->setBlendMode(BlendMode::Normal);
//    canvas->setPorterDuffMode(PorterDuff::SourceOver);
//    canvas->drawCircle(GREEN, 320, 240, 240/2);
//    canvas->drawTriangle(BLUE, 0, 300, 300, 300, 0, 0);
//    canvas->drawTriangle2(*bmp_1,0, 300,0.0,0.0, 300, 300,1.0,0.0, 0, 0,0.0,1.0);
//    canvas->drawQuad2(*bmp_1, 300, 50, 300, 300);
}



int main() {
    init_sdl(640, 480);
    loop();


}

template<typename Derived>
class Base {

    static inline void hello() {
        Derived::hello();
    }
};

class ExampleDerived : public Base<ExampleDerived> {

    static inline void hello() {
        cout << "hello";
    }
};

class ExampleDerived2 : public Base<ExampleDerived2> {

    static inline void hello() {
        cout << "hello";
    }
};



void init_sdl(int width, int height) {
    Base<ExampleDerived2> *a = new ExampleDerived2();


    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
//    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, width, height);

//    canvas = new Canvas16Bit(width, height, PixelFormat::RGB565, new RGB565_PACKED_16());
//    canvas = new Canvas24BitU8(width, height, new RGB888_ARRAY());

//    canvas = new Canvas32Bit(width, height, new RGB888_PACKED_32());
    canvas = new Canvas32Bit(width, height, new RGB888_PACKED_32());

    img_1 = resources.loadImageFromCompressedPath("charsprites.png");


    auto * bmp_1 = new Bitmap<uint32_t , RGB888_PACKED_32>(img_1.data, img_1.width, img_1.height, new RGB888_PACKED_32());
    auto * bmp_2 = new Bitmap<vec3<uint8_t>, RGB888_ARRAY>(img_1.data, img_1.width, img_1.height, new RGB888_ARRAY());

    canvas->drawQuad2(*bmp_1,0,0,0,0);
    canvas->drawQuad2(*bmp_2,0,0,0,0);

    // 24 bit with given bitmap canvas
//    auto * bmp = new Bitmap24bitU8(width, height, new RGB888_ARRAY());
//    canvas = new Canvas24BitU8(bmp);

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