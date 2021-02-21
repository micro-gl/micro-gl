
#include <iostream>
#include <chrono>
#include <SDL.h>
#include <microgl/color.h>
#include <microgl/Canvas.h>
#include <microgl/matrix_4x4.h>
#include <microgl/Q.h>
#include <microgl/camera.h>
#include <microgl/dynamic_array.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
using Canvas24 = Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24 * canvas;
sampling::flat_color color_red{{255,0,0,255}};
sampling::flat_color color_black{{0,0,0,255}};

float t = 0.0f;

void loop();
void init_sdl(int width, int height);
//template <typename number_transform, typename number_raster>
//void render_template();

template <typename number>
using arr = dynamic_array<vec3<number>>;

template <typename number>
arr<number> cube() {

    return {
            //down
            {-1, -1, -1}, // left-bottom
            {-1, -1, 1}, // left-top
            {1, -1, 1}, // right-top
            {1, -1, -1}, // right-bottom
            //up
            {-1, 1, -1}, // left-bottom
            {-1, 1, 1}, // left-top
            {1, 1, 1}, // right-top
            {1, 1, -1}, // right-bottom
    };

}

float z = 0, z2=0;

template <typename number_coords>
void render_template(const arr<number_coords> & vertices) {
    using vertex = vec3<number_coords>;
    using vertex4 = vec4<number_coords>;
    using camera = microgl::camera<number_coords>;
    using mat4 = matrix_4x4<number_coords>;
    using math = microgl::math;

    z+=0.005;
    z2+=0.005;

    int canvas_width = canvas->width();
    int canvas_height = canvas->height();
    number_coords fov_horizontal = math::deg_to_rad(60);

//    mat4 model = mat4::transform({ math::deg_to_rad(z), math::deg_to_rad(z), 0},
//                                 {0,0,-300+25}, {25,25,25});

    mat4 model = mat4::scale(100,100,100)*mat4::rotation(math::deg_to_rad(z), {0,1,1});
    mat4 view = camera::lookAt({0, 0, 1000}, {0,0, 0}, {0,1,0});
    mat4 projection = camera::perspective(fov_horizontal, canvas_width, canvas_height, 1, 10000);
//    mat4 projection = camera::orthographic(-canvas_width/2, canvas_width/2, -canvas_height/2, canvas_height/2, 1, 10000.0);
//    mat4 projection = camera::orthographic(0, canvas_width, 0, canvas_height, 1, 10000.0);
    mat4 mvp = projection * view * model;

    canvas->clear({255,255,255,255});

    for (unsigned ix = 0; ix < vertices.size(); ++ix) {
        // convert to ndc space
        vertex4 ndc_projected = mvp * vertex4{vertices[ix]};
        ndc_projected = ndc_projected/ndc_projected.w;
        // convert to raster space
        vertex raster = camera::viewport(ndc_projected, canvas_width, canvas_height);
        // perform culling
        bool inside = (raster.x >= 0) &&  (raster.x < canvas_width) &&
                (raster.y >= 0) &&  (raster.y < canvas_height) && (math::abs_(raster.z) < 1);
        if(!inside)
            continue;

//        std::cout << raster.x << ", " << raster.y << ", " << raster.z << " - " << z <<std::endl;
        canvas->drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red, color_black,
                raster.x, raster.y,
                number_coords(10), number_coords(1),
                255);
    }

}

void render() {

    render_template<float>(cube<float>());
//    render_template<Q<14>>(cube<Q<14>>());
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

        render();

        SDL_UpdateTexture(texture, nullptr, canvas->pixels(), canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}
