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
#include <microgl/tesselation/complex_to_simple_polygon.h>

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

std::vector<vec2_f> poly_hole() {
    vec2_32i p0 = {100,100};
    vec2_32i p1 = {300, 100};
    vec2_32i p2 = {300, 300};
    vec2_32i p3 = {100, 300};

    vec2_32i p4 = {150,150};
    vec2_32i p7 = {150, 250};
    vec2_32i p6 = {250, 250};
    vec2_32i p5 = {250, 150};

//    return {p4, p5, p6, p7};
    return {p0, p1, p2, p3,   p4, p7, p6, p5, p4,p3};//,p5_,p4_};
}

std::vector<vec2_f> poly_diamond() {
    vec2_f p1 = {300, 100};
    vec2_f p2 = {400, 300};
    vec2_f p3 = {300, 400};
    vec2_f p0 = {100,300};

    return {p1, p2, p3, p0};
}

dynamic_array<vec2_f> poly_inter_1() {
    vec2_f p1 = {100, 100};
    vec2_f p2 = {400, 400};
    vec2_f p3 = {100, 400};
    vec2_f p0 = {400,100};

    return {p1, p2, p3, p0};
}

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_2() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A2{
            {100,100},
            {200,100},
            {200,200},
            {100,200}
    };

    dynamic_array<vec2_f> B{
            {0.0+0,0.0+0},
            {0+300,0+0},
            {0+300,0+300},
            {0+0,0+300}
    };

    dynamic_array<vec2_f> A{
            {0,0},
            {90,0},
            {90,90},
            {0,90}
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

std::pair<dynamic_array<vec2_f>, dynamic_array<index_t>> poly_inter_deg() {
    dynamic_array<index_t> locations;

    dynamic_array<vec2_f> A{
            {400,400},
            {10,400},
            {10,10},
            {400,10},

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

std::vector<vec2_f> poly_inter_star() {
    vec2_f p0 = {150, 150};
    vec2_f p1 = {450,150};
    vec2_f p2 = {200,450};
    vec2_f p3 = {300,50};
    vec2_f p4 = {400,450};

    return {p0, p1, p2, p3, p4};
}

template <typename T>
void render_polygon(const std::pair<dynamic_array<vec2<T>>, dynamic_array<index_t>> &pieces);

void render() {
    t+=.05f;
//    std::cout << t << std::endl;
//    render_polygon<float>(poly_rect());
//    render_polygon(poly_2());
//    render_polygon(poly_hole());
//    render_polygon(poly_diamond());
//    render_polygon(poly_inter_1());
    render_polygon(poly_inter_2());
//    render_polygon(poly_inter_deg());
//    render_polygon(poly_inter_star());
//    render_polygon(poly_tri());
}


template <typename T>
void render_polygon(const std::pair<dynamic_array<vec2<T>>, dynamic_array<index_t>> &pieces) {
    using index = unsigned int;

//    polygon[3].y = 50 -  t;

    canvas->clear(WHITE);

    tessellation::complex_to_simple_polygon simplifier{true};
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

//        if(ix==0)
        canvas->drawPolygon<blendmode::Normal, porterduff::SourceOverOnOpaque, false>(
                &(simple_polygons_result.data()[offset]),
                size - 1,
                120,
                polygons::hints::SIMPLE
                );

    }

    // draw triangles batch
//    canvas->drawTriangles<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(
//            RED,
//            polygon.data(),
//            indices.data(),
//            boundary_buffer.data(),
//            indices.size(),
//            type,
//            122,
//            precision);

    return;

    // draw triangulation
//    canvas->drawTrianglesWireframe(
//            BLACK,
//            polygon.data(),
//            indices.data(),
//            indices.size(),
//            type,
//            255,
//            precision);

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