
#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
Canvas24 * canvas;
sampling::flat_color color_red{{255,0,0,255}};

float t = 0.0f;

void loop();
void init_sdl(int width, int height);
template <typename number_transform, typename number_raster>
void render_template();

void render() {
    render_template<Q<10>, Q<4>>();
//    render_template<float, float>();
//    render_template<double, double>();
}

template <typename number_transform, typename number_raster>
void render_template() {
    using vertex = vec2<number_transform>;
    using matrix_3x3_trans = matrix_3x3<number_transform>;

    t += 0.001;
    auto t_number_angle = number_transform(t);
    static float sine = 0.0f;
    sine = microgl::math::sin(t*2);
    auto number_scale = microgl::math::abs(number_transform(sine)*5);
    if (number_scale < 1.f)
        number_scale=1.f;

    matrix_3x3_trans identity = matrix_3x3_trans::identity();
    matrix_3x3_trans rotation = matrix_3x3_trans::rotation(t_number_angle);
    matrix_3x3_trans rotation_pivot = matrix_3x3_trans::rotation(t_number_angle, 50, 50, number_scale, number_scale/2);
    matrix_3x3_trans translate = matrix_3x3_trans::translate(100.0f, 100);
    matrix_3x3_trans scale = matrix_3x3_trans::scale(number_scale, number_scale);
    matrix_3x3_trans shear_x = matrix_3x3_trans::shear_x(float(t));
    matrix_3x3_trans transform = translate * rotation_pivot;

    vertex vertices[4] = {{0,0}, {100,0}, {100,100}, {0,100}};
    boundary_info boundary[2] = {create_boundary_info(true,true,false),create_boundary_info(false,true,true)};

    canvas->clear({255,255,255,255});
    canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_red,
            transform,
            vertices,
            (vec2<number_transform> *) nullptr,
            nullptr,
            boundary,
            4,
            triangles::indices::TRIANGLES_FAN,
            150);
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
    canvas = new Canvas24(width, height);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto ns = std::chrono::nanoseconds(1);
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

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}