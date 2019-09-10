
#include <stdio.h>
#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/color.h>
#include <microgl/Canvas.h>
#include <microgl/matrix.h>
#include <microgl/matrix_3x3.h>
#include <microgl/quad_matrix.h>
#include <microgl/Q.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
using Canvas24Bit_Packed32 = Canvas<uint32_t, RGB888_PACKED_32>;

Canvas24Bit_Packed32 * canvas;

Resources resources{};

void loop();
void init_sdl(int width, int height);

#define PI        3.14159265358979323846264338327950288

unsigned timer = 0;
float t = 0.0f;

inline void render() {

    Q<4> vv{};
    Q<10> a{0.3f};
    Q<10> a1 = 4;
    a1=5;
    vv = a;

    microgl::functions::sin(a1);

    using matrix_3x3_q10 = matrix_3x3<Q<10>>;
    using matrix_3x3_q4 = matrix_3x3<Q<4>>;
    using vector_3_q10 = vector<Q<10>, 3>;
//    matrix_3x3<float> aaa(0.3f);
//    matrix_3x3_q10 rotation = matrix_3x3_q10::rotation(Q<10>{float(PI)});
//    matrix_3x3_q4 identity = matrix_3x3_q4::identity();
//    matrix_3x3_q4 scale = matrix_3x3_q4::scale(2, 2);
//
//    auto i_s = identity*scale*scale;
//    auto i_r = matrix_3x3_q10{scale}*rotation;
//
//    int b = 0;

    timer++;
    t += 0.001;
//    t=-1;
    vec2_32i p0{0, 0};
    vec2_32i p1{100, 0};
    vec2_32i p2{100, 100};
    vec2_32i p3{0, 100};
//    t=PI/8.0f;
//t=0.0f;
    matrix_3x3_q10 rotation = matrix_3x3_q10::rotation(float(t));
    matrix_3x3_q10 translate = matrix_3x3_q10::translate(t,0);
    matrix_3x3_q4 identity = matrix_3x3_q4::identity();
    auto & transform = rotation;
    auto vec_0 = transform * vector_3_q10{p0.x, p0.y, 1};
    auto vec_1 = transform * vector_3_q10{p1.x, p1.y, 1};
    auto vec_2 = transform * vector_3_q10{p2.x, p2.y, 1};
    auto vec_3 = transform * vector_3_q10{p3.x, p3.y, 1};

    vec2_32i p0_t{vec_0[0].value(), vec_0[1].value()};
    vec2_32i p1_t{vec_1[0].value(), vec_1[1].value()};
    vec2_32i p2_t{vec_2[0].value(), vec_2[1].value()};
    vec2_32i p3_t{vec_3[0].value(), vec_3[1].value()};

    canvas->clear(WHITE);
    canvas->drawQuadrilateral(RED,
            p0_t.x, p0_t.y,
            p1_t.x, p1_t.y,
            p2_t.x, p2_t.y,
            p3_t.x, p3_t.y,
            10,
            255
            );
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

    canvas = new Canvas24Bit_Packed32(width, height, new RGB888_PACKED_32());

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

    SDL_DestroyWindow(window);
    SDL_Quit();
}
