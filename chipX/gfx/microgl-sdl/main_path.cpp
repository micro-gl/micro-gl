#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using microgl::tessellation::path;
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

template <typename number>
path<number> path_star() {
    path<number> path{};
    path.lineTo(150, 150).lineTo(450, 150)
            .lineTo(200,450).lineTo(300,50)
            .lineTo(400,450).closePath();
    return path;
}


template <typename number>
path<number> path_star_2() {
    path<number> path{};
    path.linesTo({{150, 150},
                  {450,150},
                  {200,450},
                  {300,50},
                  {400,450}})
            .moveTo({150/2, 150/2})
            .linesTo({{450/2,150/2},
                      {200/2,450/2},
                      {300/2,50/2},
                      {400/2,450/2}})
            .moveTo({150/10, 150/10})
            .linesTo({{450/10,150/10},
                      {200/10,450/10},
                      {300/10,50/10},
                      {400/10,450/10}})
            .rect(50, 50, 250, 250)
            .rect(50, 250, 550, 50, false)
            .rect(50, 450, 50, 50, false)
            .closePath();
    return path;
}

template <typename number>
path<number> path_rects() {
    path<number> path{};
    path
        .rect(50, 50, 250, 250, false)
        .rect(50, 250, 550, 50, true)
        .rect(50, 450, 50, 50, true);
    return path;
}

sampling::flat_color color_red {{255,0,255,255}};
sampling::flat_color color_green {{22,22,22,255}};

template <typename number>
void render_path(path<number> path) {
    canvas->clear({255, 255, 255, 255});
    canvas->drawPathFill<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_red,
            path,
            tessellation::fill_rule::even_odd
    );

//    return;
    canvas->drawPathStroke<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_green,
            path,
            number{1},
            tessellation::stroke_cap::butt,
//            tessellation::stroke_cap::round,
//            tessellation::stroke_cap::square,
            tessellation::stroke_line_join::bevel,
//            tessellation::stroke_line_join::miter,
//            tessellation::stroke_line_join::miter_clip,
//            tessellation::stroke_line_join::round,
            4, {20,10}, 0,
            255
    );
}

void render() {
    t+=.05f;

//    render_path<float>(path_star<float>());
    render_path<float>(path_star_2<float>());
//    render_path<float>(path_rects<float>());

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
