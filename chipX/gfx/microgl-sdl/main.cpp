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

vec2_32i a[3] = {{5, H - 5}, {0, 225}, {W/2, H - 5}};
vec2_32i b[7] = {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}, {W/2+W/8, H/2}, {W/2 + W/3, H/4}, {W-5, H - 5}};
vec2_32i c[5] = {{5, H - 5}, {5, 225}, {W/2, H - 5}, {W-5,255}, {W-5, H-5}};

vec2_f b_f[7] = {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}, {W/2+W/8, H/2}, {W/2 + W/3, H/4}, {W-5, H - 5}};
vec2_f c_f[5] = {{5, H - 5}, {5, 225}, {W/2, H - 5}, {W-5,255}, {W-5, H-5}};
vec2_f c2_f[3] = {{20, 400}, {20+200, 300}, {20+400, 400}};

static float d = 0;//1.0f*(float)W/3.0f;

inline void render() {

    canvas->setAntialiasing(false);
//    canvas->clear(WHITE);

    for (int ix = 0; ix < 1; ++ix) {

        canvas->clear(WHITE);
//
        static long a = 0;

        a+=20;

//        b += 10.01;//
//cout<<b<<endl;

        int G = 256;

//        canvas->drawTriangle<blendmode::Normal, porterduff::None, false, false>(
//                *bmp_uv,
//                0,0,       0, 32, 32,
//                G + b, 0,     32,32,32,
//                G,G,   32,0,32,
//                255,
//                0,5
//        );
//

//        b=0.778;
//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                *bmp_uv,
//                10.0f,                0.0f,                0.0f, 1.0f,
//                10.0f + (float)G + b, 0.0f,                1.0f, 1.0f,
//                10.0f + (float)G,     0.0f + (float)G,     1.0f, 0.0f,
//                255
//        );
//        canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                *bmp_uv,
//                10.0f + (float)G,     0.0f + (float)G,     1.0f, 0.0f,
//                10.0f           ,     0.0f + (float)G,     0.0f, 0.0f,
//                10.0f,                0.0f,                0.0f, 1.0f,
//                255
//        );

// test fill rules withs this

//b+=0.15;
//cout<<b<<endl;
//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false, sampler::NearestNeighbor>(
//                *bmp_uv,
//                0,0,          0.0, 1.0,
//                G+ d, 0,       1.0, 1.0,
//                G +0, G,      1.0, 0.0,
//                0, G,         0.0, 0.0,
//                255);

//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::None, false>(
//                RED,
//                0,0,
//                G+ 0,0,
//                G +0,G,
//                0, G,
//                255);

//        canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//                *bmp_uv,
//                10.0f + (float)G,     0.0f + (float)G,     1.0f, 0.0f,
//                10.0f           ,     0.0f + (float)G,     0.0f, 0.0f,
//                10.0f,                0.0f,                0.0f, 1.0f,
//                10.0f ,               0.0f + (float)G,     0.0f, 0.0f,
//                255);
//
static long timer = 0;
static float s;

timer++;
s = 50*sin(2.0f*3.14f*(float(timer%10000)/10000));

//b[1].y = H/4 + s;
//        d += 0.001;
        d += 0.001;

        b_f[2].y += 0.01;
        c2_f[1].y -= 0.05;
        c_f[1].x += .5;

        b[1].y -= 1;//0.05f;
//        c[1].y -= 0.05;
        canvas->drawQuadraticBezierPath(BLACK, c_f, 3, 4);
//        canvas->drawCubicBezierPath(BLACK, b, 7, 4);
//        canvas->drawLinePath(RED, b, 7);

//        canvas->drawLinePath(RED, b_f, 4);

//        canvas->drawLinePath(RED, c2_f, 3);

//        canvas->drawLine(BLACK, 20.0-0, 400.0, 220.0f+0 , 300.0f+d);
//        canvas->drawLine(BLACK, 220.0f , 300.0f-d  , 420.0f, 400.0);

//        canvas->drawLine(BLACK, 200.0f, 150.0 , 500.0f, 100.0+0 + d);
//        canvas->drawLine(BLACK, 55.0f, 55.0 , 150.0f, 400.0+0 + d);
//        canvas->drawLine(BLACK, 400.0f, 50.0+0, 200.0f, 150.0-d);

//        canvas->drawLine(BLACK, 0.0f, 200.0 , 200.0f, 200.0+d);
//        canvas->drawLine(BLACK, 200.0f, 200.0 -d, 400.0f, 100.0);

//        canvas->drawLine(BLACK, 10.0f,   240.0 , 320.0f, 10.0);
//        canvas->drawLine(BLACK, 10.0f,   240.0 , 320.0f, 240.0);
//        canvas->drawLine(BLACK, 10.0f,   10.0 + 0 , 320.0f, 240.1);
//        canvas->drawLine(BLACK, 10.0f,   10.0 + 0 , 240.0f, 520.1);




//        canvas->drawLine(BLACK, 0.0f,   100.0 + d , 200.0f, 200.1);
//        canvas->drawLine(BLACK, 200.0f, 200.0,      400.0f, 100.0 + d);





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
//    bmp_uv = new Bitmap<uint32_t , RGB888_PACKED_32>(img_2.width, img_2.height, new RGB888_PACKED_32());

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