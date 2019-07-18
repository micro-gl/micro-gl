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
#define W 640*1
#define H 480*1

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
Bitmap24bitU8 * bmp_uv_U8;
Bitmap24bit_Packed32 * bmp_uv;

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
    vec2_32i a[3] = {{5, H - 5}, {0, 225}, {W/2, H - 5}};
    vec2_32i c[5] = {{5, H - 5}, {5, 225}, {W/2, H - 5}, {W-5,255}, {W-5, H-5}};

    vec2_32i b[7] = {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}, {W/2+W/8, H/2}, {W/2 + W/3, H/4}, {W-5, H - 5}};

    for (int ix = 0; ix < 1; ++ix) {
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


        canvas->clear(WHITE);
//        canvas->drawQuad<blendmode::Normal, porterduff::None>(GREEN, 0, 0, 640, 480);
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

        // rotated cube
//        canvas->drawTriangle<blendmode::Normal, porterduff::None, false>(
//                *bmp_2,
//                0, W/3, 0.0, 0.0,
//                W/3, 0, 0.0, 1.0,
//                2*W/3, W/3, 1.0, 1.0,
//                255);
//
        static long a = 0;
        static float b = 2.0f*(float)W/3.0f;

        a++;
//
        b = b + (float)1/10.0f;

//        cout << b <<endl;

        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
                *bmp_uv,
                (float)2*W/3, (float)W/3, 1.0, 1.0,
                (float)W/3, (float)b, 1.0, 0.0,
                (float)0, (float)W/3, 0.0, 0.0 ,
                255);

//
//        canvas->drawTriangle<blendmode::Normal, porterduff::None, false>(
//                RED,
//                (int)0, (int)W/3,
//                (int)W/3, (int)0,
//                (int)2*W/3, (int)W/3,
//                255);


//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                RED,
//                2*(float)W/3, (float)W/3,
//                (float)W/3, (float)5*W/3,
//                (float)0, (float)2*W/3);

        // filled Quadrilateral
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false>(
//                RED,
//                0, W/3,
//                W/3, 0,
//                2*W/3, W/3,
//                W/3, 2*W/3,
//                255);
//
        // texture mapped
        int G = 600;
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false>(
//                *bmp_uv,
//                20,   20,   0.0, 1.0,
//                20+G, 20,   1.0, 1.0,
//                20+G, 20+G, 1.0, 0.0,
//                20,   20+G, 0.0, 0.0,
//                255);
//
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                *bmp_uv,
//                4*W/10,   80,   0.0, 1.0,
//                6*W/10, 80,   1.0, 1.0,
//                W-80,     H-80, 1.0, 0.0,
//                4*W/10,    H-80, 0.0, 0.0,
//                255);

// test fill rules withs this
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                *bmp_uv,
//                0, W/3, 0.0, 0.0,
//                W/3, 0, 0.0, 1.0,
//                2*W/3, W/3, 1.0, 1.0,
//                W/3, 2*W/3, 1.0, 0.0,
//                128);


//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, false>(
//                *bmp_2,
//                4*W/10,   80,   0.0, 1.0,
//                6*W/10, 80,   1.0, 1.0,
//                W-80,     H-80, 1.0, 0.0,
//                80,    H-80, 0.0, 0.0,
//                128);
//
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                *bmp_uv,
//                80,   80,   0.0, 1.0,
//                W/2,  H/4,   1.0, 1.0,
//                W/2,   2*H/4, 1.0, 0.0,
//                80,   H-80, 0.0, 0.0,
//                255);

//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false>(
//                *bmp_uv,
//                80,      80,   0.0, 1.0,
//                400,      80,   1.0, 1.0,
//                400,   400, 1.0, 0.0,
//                80,       400, 0.0, 0.0,
//                255);
//
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false>(
//                *bmp_uv,
//                400,      80,   0.0, 1.0,
//                600,      80,   1.0, 1.0,
//                280,   400, 1.0, 0.0,
//                80,       400, 0.0, 0.0,
//                255);
//

//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false>(
//                *bmp_2,
//                0, 0, 0.0, 1.0,
//                W, 0, 1.0, 1.0,
//                W, H, 1.0, 0.0,
//                0, H, 0.0, 0.0,
//                255);


//        canvas->drawQuadraticBezierPath(BLACK, c, 3);
//        canvas->drawCubicBezierPath(BLACK, b, 7);
//        canvas->drawLinePath(RED, b, 7);
//        canvas->drawCircle(RED, 320, 240, 200);
//        canvas->drawCircleFPU<blendmode::Normal, porterduff::SourceOver>(RED, 320, 240, 200,1.0f);
//        canvas->drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(RED, 320, 240, 200);

// started 150, then 80 with coding (and 32 without coding)
//        canvas->drawTriangle<blendmode::Normal, porterduff::None>(RED, 0, 0,
//                                                                                W, H,
//                                                                                0, H, 255);
//
//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, false>(BLACK, 80, 80,
//                                                                                W-80, H-80,
//                                                                                80, 280, 255);

//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(RED, 80, 80,
//                                                                                W-80, H-80,
//                                                                                80, 280, 255);

//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque>(RED, 80, H-280,
//                                                                                W-80, 80,
//                                                                                80, H-80,255);

//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque>(GREEN, 80, 80,
//        W/4, H/4,
//        80, H, 255);

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
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");


//    auto * bmp_1 = new Bitmap<uint32_t , RGB888_PACKED_32>(img_1.data, img_1.width, img_1.height, new RGB888_PACKED_32());
    bmp_1 = new Bitmap<vec3<uint8_t>, RGB888_ARRAY>(img_1.data, img_1.width, img_1.height, new RGB888_ARRAY());
    bmp_2 = bmp_1->convertToBitmap<uint32_t , RGB888_PACKED_32>();

    bmp_uv_U8 = new Bitmap<vec3<uint8_t>, RGB888_ARRAY>(img_2.data, img_2.width, img_2.height, new RGB888_ARRAY());
    bmp_uv = bmp_uv_U8->convertToBitmap<uint32_t , RGB888_PACKED_32>();

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
        cout<<"render"<< endl;
        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }


//    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_Quit();
}