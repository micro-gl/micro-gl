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
path<number> path_star() {
    path<number> path{};
    path.lineTo({150, 150}).lineTo({450, 150})
            .lineTo({200,450}).lineTo({300,50})
            .lineTo({400,450}).closePath();
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

float t = 0;

template <typename number>
path<number> path_arc() {
    path<number> path{};
    int div=3; //4
    path.arc({200,200}, 100,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div).closePath();
//
    path.arc({250,200}, 50,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             true, div).closePath();
t+=0.82f;
//    t=69.6999817f;//819999992f;//-0.01f;
////t=26.0399914;
    path.moveTo({150,150});
    path.arc({150+0,150}, 50+t-0,
             math::deg_to_rad(0.0f),
             math::deg_to_rad(360.0f),
             false, div);//.closePath();

//             // case 1
//    path.moveTo({150,150});
//    path.arc({150+t,150}, 50+t-0,
//             math::deg_to_rad(0.0f),
//             math::deg_to_rad(360.0f),
//             false, div);//.closePath();

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

template <typename number>
path<number> path_test() {
    path<number> path{};
    int div=8;
//    t+=0.01;
    t=137.999039f;
    path.linesTo({{100,100}, {300,100}, {300, 300}, {100,300}});
    vec2<number> start{22.0f, 150.0f-0.002323204};
    path.moveTo(start);
    path.linesTo({start, {300,120.002323204-t}, {300, 300}, {100,300}});
    path.moveTo({200, 200});
    path.lineTo({300,10});

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
            tessellation::fill_rule::even_odd,
            tessellation::tess_quality::prettier_with_extra_vertices,
            50
    );

    return;
    canvas->drawPathStroke<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_green,
            path,
            number{10},
            tessellation::stroke_cap::butt,
//            tessellation::stroke_cap::round,
//            tessellation::stroke_cap::square,
            tessellation::stroke_line_join::bevel,
//            tessellation::stroke_line_join::miter,
//            tessellation::stroke_line_join::miter_clip,
//            tessellation::stroke_line_join::round,
            4, {0,0}, 0,
            44
    );
}

void render() {
//    t+=.05f;

//    render_path<float>(path_star<float>());
//    render_path<float>(path_star_2<float>());
//    render_path<float>(path_rects<float>());
//    render_path<float>(path_arc<float>());
//    render_path<double>(path_arc<double>());
    render_path<float>(path_arc<float>());
//    render_path<Q<15>>(path_arc<Q<15>>());
//    render_path<Q<4>>(path_arc<Q<4>>());
//    render_path<double>(path_test<double>());

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
