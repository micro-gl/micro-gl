
#include <iostream>
#include <chrono>
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
    using vector_3_transform = column_vector<number_transform, 3>;
    using vector_3_raster = column_vector<number_raster, 3>;

    t += 0.001;
    auto t_number_angle = number_transform(t);
    static float sine = 0.0f;
    sine = microgl::math::sin(t*10);
    auto number_scale = microgl::math::abs_(number_transform(sine)*2);
    if (number_scale < 1.f)
        number_scale=1.f;
//    number_scale =5.0f;

    vertex p0{0, 0};
    vertex p1{100, 0};
    vertex p2{100, 100};
    vertex p3{0, 100};

    matrix_3x3_trans identity = matrix_3x3_trans::identity();
    matrix_3x3_trans rotation = matrix_3x3_trans::rotation(t_number_angle);
    matrix_3x3_trans rotation_pivot = matrix_3x3_trans::rotation(t_number_angle, 50, 50, number_scale, number_scale);
    matrix_3x3_trans translate = matrix_3x3_trans::translate(100.0f, 100);
    matrix_3x3_trans scale = matrix_3x3_trans::scale(number_scale, number_scale);
    matrix_3x3_trans shear_x = matrix_3x3_trans::shear_x(float(t));
    matrix_3x3_trans transform = translate  * rotation_pivot;

    // this also converts into the raster precision :-) with
    // the conversion constructor
    vector_3_raster vec_0 = transform * vector_3_transform{p0.x, p0.y, 1};
    vector_3_raster vec_1 = transform * vector_3_transform{p1.x, p1.y, 1};
    vector_3_raster vec_2 = transform * vector_3_transform{p2.x, p2.y, 1};
    vector_3_raster vec_3 = transform * vector_3_transform{p3.x, p3.y, 1};

    canvas->clear(WHITE);
    canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number_raster>(
            RED,
            vec_0[0], vec_0[1],
            vec_1[0], vec_1[1],
            vec_2[0], vec_2[1],
            150,
            true, true, false
    );

    canvas->drawTriangle<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number_raster>(
            RED,
            vec_2[0], vec_2[1],
            vec_3[0], vec_3[1],
            vec_0[0], vec_0[1],
            150,
            true, true, false
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
        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
