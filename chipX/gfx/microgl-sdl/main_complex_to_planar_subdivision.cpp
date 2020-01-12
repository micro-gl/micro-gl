#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/PixelCoder.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/planarize_division.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using namespace tessellation;
using index_t = unsigned int;
using Canvas24Bit_Packed32 = Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24Bit_Packed32 * canvas;

void loop();
void init_sdl(int width, int height);

float t = 0;

dynamic_array<vec2_f> box(float left, float top, float right, float bottom, bool ccw=false) {
    if(!ccw)
        return {
                {left,top},
                {right,top},
                {right,bottom},
                {left,bottom},
        };

    return{
            {left,top},
            {left,bottom},
            {right,bottom},
            {right,top},
    };
};

chunker<vec2_f> poly_inter_star() {
    chunker<vec2_f> A;

    A.push_back_and_cut({
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    A.push_back_and_cut(box(50,50,300,300, true));

    return A;
}

chunker<vec2_f> poly_inter_simple_1() {
    chunker<vec2_f> A;

//    A.push_back_and_cut({
//                                {20,20},
//                                {400,20},
//                                {200,400},
//                        });

    A.push_back_and_cut({
                                {100,50},
                                {400,50},
                                {500,400},
//                                {120,100},
                        });

    return A;
}

chunker<vec2_f> box_1() {
    chunker<vec2_f> A;

//    A.push_back_and_cut({
//                                {20,20},
//                                {400,20},
//                                {200,400},
//                        });

    A.push_back_and_cut(box(50,50,300,300));
    A.push_back_and_cut(box(150,150,200,200));

    return A;
}

template <typename T>
void render_polygon(chunker<vec2<T>> pieces) {
    using index = unsigned int;
    using psd = tessellation::planarize_division<T>;

    dynamic_array<vec2<T>> trapezes;

    canvas->clear(color::colors::WHITE);

    psd::compute_DEBUG(pieces, trapezes);

    for (index ix = 0; ix < trapezes.size(); ix+=4) {
//        canvas->drawPolygon(&trapezes[ix], 4, )
        canvas->drawLinePath(color::colors::BLACK,
                &trapezes[ix], 4, true);
    }

//        canvas->drawQuad(RED, 0, 0, 100,100, 0,255);
//    canvas->drawPolygon<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//            chunk.data,
//            chunk.size,
//            120,
//            polygons::hints::SIMPLE
//    );


}

void render() {
    t+=.05f;

    render_polygon(poly_inter_star());
//    render_polygon(box_1());
//    render_polygon(poly_inter_simple_1());
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

    canvas = new Canvas24Bit_Packed32(width, height);
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
