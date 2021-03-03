#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL.h>
#include <microgl/buffer.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/experiments/bentley_ottmann/Segment.h>
#include <microgl/tesselation/experiments/bentley_ottmann/BentleyOttmannLineIntersection.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
using Canvas24= canvas<Bitmap24>;
using namespace experiments;
Canvas24 * canvas;
microgl::sampling::flat_color color_RED {{255,0,0,255}};
Resources resources{};

void loop();
void init_sdl(int width, int height);

using namespace tessellation;

void render_star();
void render_chaos();
float t = 20;

void render() {
//    render_star();
    render_chaos();
}

void render_star() {

    Segment s1 {{100,100}, {300, 100}};
    Segment s2 {{300,100}, {120, 300}};
    Segment s3 {{120,300}, {200, 20}};
    Segment s4 {{200, 20}, {280,300}};
    Segment s5 {{280,300}, {100, 100}};

    canvas->clear({255,255,255,255});

    std::vector<Segment> segments_2 {s1, s2, s3, s4, s5};

    auto & segments = segments_2;
    //cout << t <<endl;
    BentleyOttmann bentleyOttmann(true);
    uint8_t precision = 0;
    auto & I = bentleyOttmann.compute(segments.data(), segments.size(), precision);

    for (auto & inter : I) {
        canvas->drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_RED, color_RED, inter.x, inter.y, 8, 1, 80);
    }

    for (auto & segment : segments) {
        canvas->drawWuLine({0,0,0,255},
                         segment.p0.x, segment.p0.y,
                         segment.p1.x, segment.p1.y,
                         0
        );

    }


}


void render_chaos() {
    int m = 0;

    Segment s1 {{100,20+m}, {100, 400+m}};
    Segment s2 {{200,20+m}, {200, 400+m}};
    Segment s6 {{300,20+m}, {300, 400+m}};

    Segment s3 {{20,200+m}, {400, 200+m}};
    Segment s4 {{20,300+m}, {400, 300+m}};
    Segment s5 {{20,350+m }, {400, 350+m}};

    Segment s7 {{110,50+m}, {180, 400+m}};
    Segment s8 {{500,5+m}, {130, 400+m}};

    canvas->clear({255,255,255,255});
//    t -=0.25;
    t -=0.2;
    //s7.p0.x = 15 + t;
    //s8.p0.x = 400 - t;

    s7.p0.x = 600+t;
    s8.p0.x = 10-t;
    s7.p0.y = 10;
    s8.p0.y = 10;

    float s = -t;
    s3.p1.y = 200 + m + 101*sin(6.0-(s/10));
    s4.p1.y = 300 + m + 100*sin(6.0-(s/20));
    s5.p1.y = 350 + m + 100*sin(6.0-(s/25));

    s1.p1.x = 100 + m + 101*sin(6.0-(t/10));
    s2.p1.x = 200 + m + 100*sin(6.0-(t/20));
    s6.p1.x = 300 + m + 100*sin(6.0-(t/25));

    std::vector<Segment> segments_2 {s1, s2, s3, s4, s5, s6, s7, s8};
    //std::vector<Segment> segments_2 {s2,s4, s7};

    auto & segments = segments_2;
    //cout << t <<endl;
    BentleyOttmann bentleyOttmann(true);
    uint8_t precision = 8;
    auto & I = bentleyOttmann.compute(segments.data(), segments.size(), precision);

    for (auto & inter : I) {
        canvas->drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_RED, color_RED, inter.x, inter.y, 8, 1, 80);
    }

    for (auto & segment : segments) {
        canvas->drawWuLine({0,0,0,255},
                         segment.p0.x, segment.p0.y,
                         segment.p1.x, segment.p1.y,
                         0
        );

    }


}


int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Bentley Ottmann", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STATIC, width, height);

    canvas = new Canvas24(width, height);

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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

#pragma clang diagnostic pop