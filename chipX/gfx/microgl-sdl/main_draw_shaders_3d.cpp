#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/camera.h>
#include <microgl/Canvas.h>
#include <microgl/PixelCoder.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/Bilinear.h>
#include <microgl/shaders/color_shader.h>
#include <microgl/shaders/texture_shader.h>
#include "data/model_3d_cube.h"

using namespace microgl::shading;
#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
Resources resources{};

using namespace microgl::shading;
using index_t = unsigned int;
using Canvas24Bit_Packed32 = Canvas<uint32_t, coder::RGB888_PACKED_32>;

Canvas24Bit_Packed32 * canvas;

void loop();
void init_sdl(int width, int height);

float t = 0;

// bitmap for mapping
Bitmap<uint32_t, coder::RGB888_PACKED_32> *bmp_uv;

template <typename number>
void test_shader_color_2d() {
    color_shader<number> shader;
    shader.mat= camera<number>::orthographic(0, W, 0, H, 1, 100);

    color_shader_vertex_attributes<number> v0, v1, v2;
    v0.point= {10.0,10.0, 0};  v0.color= {255,0,0,255};
    v1.point= {400.0,200.0, 0}; v1.color= {0,255,0,255};
    v2.point= {10.0,400.0, 0}; v2.color= {0,0,255,255};

    canvas->drawTriangleShader<blendmode::Normal, porterduff::None, false>(shader, v0, v1, v2, 255);
//    canvas->drawTriangle(color::colors::RED, 10.0,10.0, 400.0,10.0, 400.0,400.0, 255);
}
float z=0.0;
template <typename number>
void test_shader_texture_2d(const model_3d<number> & object) {

    using vertex = vec3<number>;
    using camera = microgl::camera<number>;
    using mat4 = matrix_4x4<number>;
    using math = microgl::math;
    using vertex_attribute= texture_shader_vertex_attributes<number>;

    z+=0.1121;

    int canvas_width = canvas->width();
    int canvas_height = canvas->height();

    mat4 model = mat4::transform({ 0, math::deg_to_rad(z), math::deg_to_rad(0/2)},
                                 {0,0,0}, {100,100,100});
//    mat4 view = camera::lookAt({0, 0, -z}, {0,0, -z-1}, {0,1,0});
    mat4 view = camera::lookAt({0, 0, 600}, {0,0, 0}, {0,1,0});
    mat4 projection = camera::perspective(math::deg_to_rad(60),
                                          canvas_width, canvas_height, 1, 500);
    mat4 mvp= projection*view*model;
    texture_shader<number, uint32_t, coder::RGB888_PACKED_32, sampler::NearestNeighbor> shader;
    shader.matrix= mvp;//camera<number>::orthographic(0, W, 0, H, 1, 100);
    shader.texture= bmp_uv;

    // model to vertex buffers
    dynamic_array<vertex_attribute> vertex_buffer{object.vertices.size()};
    for (unsigned ix = 0; ix < object.vertices.size(); ++ix) {
        vertex_attribute v{};
        v.point= object.vertices[ix];
        v.uv= object.uvs[ix];
        vertex_buffer.push_back(v);
    }

    canvas->drawTriangles<blendmode::Normal, porterduff::None, false, false>(
            shader,
            vertex_buffer.data(),
            object.indices.data(),
            nullptr,
            object.indices.size(),
            object.type);

//    vertex_attribute v0, v1, v2;
//    v0.point= {10.0,10.0, 0};   v0.uv= {0.0f, 0.0f};
//    v1.point= {400.0,10.0, 0};  v1.uv= {1.0f, 0.0f};
//    v2.point= {400.0,400.0, 0}; v2.uv= {1.0f, 1.0f};
//
//    canvas->drawTriangleShader<blendmode::Normal, porterduff::None, false>(shader, v0, v1, v2, 255);
//    canvas->drawTriangle<blendmode::Normal, porterduff::None>(*bmp_uv,
//            10.0,10.0, 0.0, 0.0,
//            400.0,10.0, 1.0, 0.0,
//            400.0,400.0, 1.0, 1.0,
//            255);
}

void render() {
    canvas->clear(color::colors::WHITE);

//    test_shader_color_2d<float>();
//    test_shader_color_2d<Q<10>>();
    test_shader_texture_2d<float>(cube_3d<float>);
//    test_shader_texture_2d<float>();

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
    auto img_2 = resources.loadImageFromCompressedPath("uv_256.png");
    auto bmp_uv_U8 = new Bitmap<vec3<uint8_t>, coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    bmp_uv = bmp_uv_U8->convertToBitmap<uint32_t , coder::RGB888_PACKED_32>();

    canvas = new Canvas24Bit_Packed32(width, height);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        render();
    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return int_ms.count();}

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
