#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/planarize_division.h>
#include <microgl/tesselation/path.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using index_t = unsigned int;
using Canvas24 = Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24 * canvas;

void loop();
void init_sdl(int width, int height);

float t = 0;

template <typename number>
dynamic_array<vec2<number>> box(float left, float top, float right, float bottom, bool ccw=false) {
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

template <typename number>
chunker<vec2<number>> poly_inter_star() {
    chunker<vec2<number>> A;

    A.push_back_and_cut({
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    return A;
}


template <typename number>
chunker<vec2<number>> poly_inter_star_2() {
    chunker<vec2<number>> A;

    A.push_back_and_cut({
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    A.push_back_and_cut({
                                {150/2, 150/2},
                                {450/2,150/2},
                                {200/2,450/2},
                                {300/2,50/2},
                                {400/2,450/2},
                        });

    A.push_back_and_cut({
                                {150/10, 150/10},
                                {450/10,150/10},
                                {200/10,450/10},
                                {300/10,50/10},
                                {400/10,450/10},
                        });

    A.push_back_and_cut(box<number>(50,50,300,300, false));
    A.push_back_and_cut(box<number>(50,250,600,300, true));
    A.push_back_and_cut(box<number>(50,450,100,500, true));

    return A;
}

template <typename number>
chunker<vec2<number>> box_1() {
    chunker<vec2<number>> A;

//    A.push_back_and_cut({
//                                {20,20},
//                                {400,20},
//                                {200,400},
//                        });

    A.push_back_and_cut(box<number>(50,50,300,300));
//    A.push_back_and_cut(box<number>(150,150,200,200));

    return A;
}


sampling::flat_color color_red {{255,0,255,255}};

template <typename number>
void render_polygon(chunker<vec2<number>> pieces) {
    using index = unsigned int;
    using psd = microgl::tessellation::planarize_division<number>;

    canvas->clear({255, 255, 255, 255});

//    psd::compute_DEBUG(pieces, trapezes);
    dynamic_array<vec2<number>> trapezes;
    dynamic_array<vec2<number>> vertices;
    dynamic_array<index> indices;
    dynamic_array<triangles::boundary_info> boundary;
    triangles::indices type;
    psd::compute(pieces, tessellation::fill_rule::even_odd, tessellation::tess_quality::fine,
            vertices, type, indices, &boundary, &trapezes);

    canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_red,
            vertices.data(),
            (vec2<number> *)nullptr,
            indices.data(),
            boundary.data(),
            indices.size(),
            type,
            50);

//        return;

    canvas->drawTrianglesWireframe({0,0,0,255},
            vertices.data(),
            indices.data(),
            indices.size(),
            type,
            40);

    for (index ix = 0; ix < trapezes.size(); ix+=4) {
        canvas->drawWuLinePath({0,0,0,255},
                &trapezes[ix], 4, true);
    }

}

void render() {
    t+=.05f;

//    render_polygon<float>(box_1<float>());
    render_polygon<float>(poly_inter_star<float>());
//    render_polygon<float>(poly_inter_star_2<float>());

//    render_polygon(poly_inter_star());
//    render_polygon<double>(poly_inter_star_2<double>());
//    render_polygon<float>(poly_inter_star_2<float>());
//    render_polygon<Q<0>>(poly_inter_star_2<Q<0>>());
//    render_polygon<Q<4>>(poly_inter_star<Q<4>>());

//    render_polygon<Q<0>>(poly_inter_star<Q<0>>());
//    render_polygon<Q<10>>(poly_inter_star<Q<10>>());

//    render_polygon<Q<0>>(poly_inter_star_2<Q<0>>());
//    render_polygon<Q<1>>(poly_inter_star_2<Q<1>>());
//    render_polygon<Q<2>>(poly_inter_star_2<Q<2>>());
//    render_polygon<Q<3>>(poly_inter_star_2<Q<3>>());
//    render_polygon<Q<4>>(poly_inter_star_2<Q<4>>());
//    render_polygon<Q<8>>(poly_inter_star_2<Q<8>>());
//    render_polygon<Q<12>>(poly_inter_star_2<Q<12>>());
//    render_polygon<Q<15>>(poly_inter_star_2<Q<15>>());
//    render_polygon<Q<16>>(poly_inter_star_2<Q<16>>());
//    render_polygon<Q<17>>(poly_inter_star_2<Q<17>>());
//    render_polygon<Q<18>>(poly_inter_star_2<Q<18>>());
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

    canvas = new Canvas24(width, height);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        render();
    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return int_ms.count();
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
