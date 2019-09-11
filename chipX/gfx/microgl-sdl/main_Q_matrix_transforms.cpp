
#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/color.h>
#include <microgl/Canvas.h>
#include <microgl/matrix_3x3.h>
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

    Q<0> a{1024};
    Q<10> b{0.5f};
    Q<0> cc =  a*b;
    Q<0> cc2 =  a/b;

    using q_trans = Q<10>;
    using q_raster = Q<12>;
    using matrix_3x3_q_trans = matrix_3x3<q_trans>;
    using matrix_3x3_q4 = matrix_3x3<Q<4>>;
    using vector_3_q = vector<q_trans, 3>;
    using vector_3_q_raster = vector<q_raster, 3>;
    using precision_t = unsigned;

    precision_t precision_transform = q_trans::precision;
    precision_t precision_rasterizer = q_raster::precision;

    timer++;
    t += 0.001;

    vec2_32i p0{0, 0};
    vec2_32i p1{100, 0};
    vec2_32i p2{100, 100};
    vec2_32i p3{0, 100};

    matrix_3x3_q_trans rotation = matrix_3x3_q_trans::rotation(float(t));
    matrix_3x3_q_trans rotation_pivot = matrix_3x3_q_trans::rotation(float(t), 50, 50);
    matrix_3x3_q_trans translate = matrix_3x3_q_trans::translate(100.0f,100);
    matrix_3x3_q_trans scale = matrix_3x3_q_trans::scale(3.0f,3.0f);
    matrix_3x3_q_trans shear_x = matrix_3x3_q_trans::shear_x(float(t));
    matrix_3x3_q4 identity = matrix_3x3_q4::identity();
//    matrix_3x3_q_trans transform = rotation*translate*scale;
    matrix_3x3_q_trans transform = translate*scale*rotation_pivot;
//    matrix_3x3_q_trans transform = shear_x;

    // this also converts into the raster precision :-) with
    // the conversion constructor
    vector_3_q_raster vec_0 = transform * vector_3_q{p0.x, p0.y, 1};
    vector_3_q_raster vec_1 = transform * vector_3_q{p1.x, p1.y, 1};
    vector_3_q_raster vec_2 = transform * vector_3_q{p2.x, p2.y, 1};
    vector_3_q_raster vec_3 = transform * vector_3_q{p3.x, p3.y, 1};

    vec2_32i p0_t{vec_0[0].value(), vec_0[1].value()};
    vec2_32i p1_t{vec_1[0].value(), vec_1[1].value()};
    vec2_32i p2_t{vec_2[0].value(), vec_2[1].value()};
    vec2_32i p3_t{vec_3[0].value(), vec_3[1].value()};

    canvas->clear(WHITE);
    canvas->drawQuadrilateral<blendmode::Normal, porterduff::SourceOverOnOpaque, false>(
            RED,
            p0_t.x, p0_t.y,
            p1_t.x, p1_t.y,
            p2_t.x, p2_t.y,
            p3_t.x, p3_t.y,
            precision_rasterizer,
            111
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
