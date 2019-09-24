#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <iostream>
#include <chrono>
#include "src/Resources.h"
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/vec2.h>
#include <microgl/PixelCoder.h>
#include <microgl/dynamic_array.h>
#include <microgl/static_array.h>
#include <microgl/tesselation/simplifier.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

typedef Canvas<uint32_t, RGB888_PACKED_32> Canvas24Bit_Packed32;

Canvas24Bit_Packed32 * canvas;

Resources resources{};

void loop();
void init_sdl(int width, int height);

using namespace tessellation;
using index_t = unsigned int;

float t = 0;

std::vector<vec2_f> poly_rect() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {300, 300};
    vec2_32i p3 = {100, 300};

    return {p0, p1, p2, p3};
}

float b = 1;
std::vector<vec2_f> poly_2() {
    vec2_f p0 = {100/b,100/b};
    vec2_f p1 = {300/b, 100/b};
    vec2_f p2 = {300/b, 300/b};
    vec2_f p3 = {200/b, 200/b};
    vec2_f p4 = {100/b, 300/b};

    return {p0, p1, p2, p3, p4};
}

std::vector<vec2_f> poly_tri() {
    vec2_f p0 = {100,100};
    vec2_f p3 = {300, 100};
    vec2_f p4 = {100, 300};

    return {p0, p3, p4};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_degenerate_hole() {
    dynamic_array<vec2_f> A{
            {100,100},
            {300,100},
            {300,300},
            {100,300},

            {150,150},
            {150,250},
            {250,250},
            {250,150},

            {150,150},
            {100,300},

    };

    dynamic_array<index_t> locations = {0, A.size()};

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_degenerate_multipepoints() {
    dynamic_array<vec2_f> A{
            {100,100},
            {100,100},
            {100,100},
            {100,100},

            {300,100},
            {300,100},
            {300,100},
            {300,100},

            {300,300},
            {300,300},
            {300,300},
            {300,300},

            {100,300},
            {100,300},
            {100,300},
            {100,300},

    };

    dynamic_array<index_t> locations = {0, A.size()};

    return {A, locations};
}

std::vector<vec2_f> poly_diamond() {
    vec2_f p1 = {300, 100};
    vec2_f p2 = {400, 300};
    vec2_f p3 = {300, 400};
    vec2_f p0 = {100,300};

    return {p1, p2, p3, p0};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_1() {
    dynamic_array<vec2_f> A{
            {100,100},
            {400,400},
            {100,400},
            {400,100}
    };

    dynamic_array<index_t> locations = {0, A.size()};

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_2() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A2{
            {100,100},
            {200,100},
            {200,200},
            {100,200}
    };

    dynamic_array<vec2_f> A{
            {0.0,0.0},
            {50,0},
            {50,50},
            {0,50},
    };

    dynamic_array<vec2_f> B{
            {50+0.0,50+0.0},
            {50+50,0+50},
            {50+50,50+50},
            {0+50,50+50},
    };

    locations.push_back(0);
    locations.push_back(A.size());
    locations.push_back(A.size() + B.size());

//    locations.push_back(A.size() + B.size() + C.size());

    A.push_back(B);
//    A.push_back(C);

//    dynamic_array<vec2_f> C = A;

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_side() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {0.0,0.0},
            {400,0},
            {400,400},
            {0,400},
    };

    dynamic_array<vec2_f> B{
            {0.0,0.0},
            {400,0},
            {400,400},
            {0,400},
    };

    locations.push_back(0);
    locations.push_back(A.size());
    locations.push_back(A.size() + B.size());

//    locations.push_back(A.size() + B.size() + C.size());

    A.push_back(B);
//    A.push_back(C);

//    dynamic_array<vec2_f> C = A;

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_nested_3() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {0.0,0.0},
            {300,0},
            {300,300},
            {0,300},
    };

    dynamic_array<vec2_f> B{
            {0,50+0.0},
            {300+50,0+50},
            {300+50,300-50},
            {0,300-50},
    };

    dynamic_array<vec2_f> C{
            {100+0.0,100+0.0},
            {300+100,0+100},
            {300+100,300-100},
            {0+100,300-100},
    };

    dynamic_array<vec2_f> D{
            {10,150},
            {500,150},
            {500,170},
            {10,170},
    };

    locations.push_back(0);
    locations.push_back(A.size());
    locations.push_back(A.size() + B.size());
    locations.push_back(A.size() + B.size() + C.size());
    locations.push_back(A.size() + B.size() + C.size() + D.size());

    A.push_back(B);
    A.push_back(C);
    A.push_back(D);

//    dynamic_array<vec2_f> C = A;

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_nested_2() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
//            {0.0,0.0},
//            {300,0},
//            {300,300},
//            {0,300},
    };

    dynamic_array<vec2_f> B{
            {0,50.},
            {250,50},
            {250,250},
            {0,250},
    };

    dynamic_array<vec2_f> C{
            {100,100},
            {400,100},
            {400,200},
            {100,200},
    };

    dynamic_array<vec2_f> D{
            {10,150},
            {300,150},
            {300,170},
            {10,170},
    };

    locations.push_back(0);
    locations.push_back(A.size());
    locations.push_back(A.size() + B.size());
    locations.push_back(A.size() + B.size() + C.size());
    locations.push_back(A.size() + B.size() + C.size() + D.size());

    A.push_back(B);
    A.push_back(C);
    A.push_back(D);

    return {A, locations};
}


std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_weird_touch() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {50.0,50.0},
            {300,50},
            {300,350},
            {100,350},

            {100,50},
    };

    dynamic_array<vec2_f> B{
            {50.0,350.0},
            {300,350},
            {300,50},
            {100,50},

            {100,350},
    };

    locations.push_back(0);
    locations.push_back(A.size());

    return {B, locations};
}


std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_nested_disjoint() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {0.0,0.0},
            {300,0},
            {300,300},
            {0,300},
    };

    dynamic_array<vec2_f> B{
            {50,50.},
            {50,250},
            {400,250},
            {400,50},
    };

    dynamic_array<vec2_f> C{
            {350,100.},
            {500,100},
            {500,150},
            {350,150},
    };

    locations.push_back(0);
    locations.push_back(A.size());
    locations.push_back(A.size() + B.size());
    locations.push_back(A.size() + B.size() + C.size());

    A.push_back(B);
    A.push_back(C);

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_deg() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {400,400},
            {0,400},
            {0,0},
            {400,0},

            {50,350},
            {100,350},
            {100,100},

    };

    locations.push_back(0);
    locations.push_back(A.size());
//    locations.push_back(A.size() + B.size());

//    A.push_back(B);
//    dynamic_array<vec2_f> C = A;

    return {A, locations};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_star() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {150, 150},
            {450,150},
            {200,450},
            {300,50},

            {400,450},

    };

    locations.push_back(0);
    locations.push_back(A.size());

    return {A, locations};
}

template <typename T>
void render_polygon(const std::pair<dynamic_array<vec2<T>>, dynamic_array<index_t>> &pieces);

void render() {
    t+=.05f;
//    std::cout << t << std::endl;
//    render_polygon<float>(poly_rect());
//    render_polygon(poly_2());

//    render_polygon(poly_degenerate_hole());

//    render_polygon(poly_degenerate_multipepoints());

//    render_polygon(poly_diamond());
//    render_polygon(poly_inter_1());

//    render_polygon(poly_inter_weird_touch());

//    render_polygon(poly_inter_nested_3());
//    render_polygon(poly_inter_nested_disjoint());
//    render_polygon(poly_inter_nested_2());

//    render_polygon(poly_inter_2());

//    render_polygon(poly_inter_side());

//    render_polygon(poly_inter_deg());

    render_polygon(poly_inter_star());

//    render_polygon(poly_tri());
}


template <typename T>
void render_polygon(const std::pair<dynamic_array<vec2<T>>, dynamic_array<index_t>> &pieces) {
    using index = unsigned int;

//    polygon[3].y = 50 -  t;

    canvas->clear(WHITE);

    tessellation::simplifier simplifier{true};
    vector<int> winding;
    static_array<vec2<T>, 256> simple_polygons_result;
    static_array<index, 256> simple_polygons_locations;

    simplifier.compute(
            pieces.first.data(),
            pieces.second,
            simple_polygons_result,
            simple_polygons_locations,
            winding);

    for (index ix = 0; ix < simple_polygons_locations.size()-1; ++ix) {
        index offset = simple_polygons_locations[ix];
        index size = simple_polygons_locations[ix+1] - offset;

//        if(ix!=0)
//            continue;

        canvas->drawPolygon<blendmode::Normal, porterduff::SourceOverOnOpaque, false>(
                &(simple_polygons_result.data()[offset]),
                size - 1,
                120,
                polygons::hints::SIMPLE
                );

//        if(false)
//        canvas->drawLinePath(
//        BLACK,
//        dd.data(),
//        dd.size(),
//        false);

    }

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

#pragma clang diagnostic pop