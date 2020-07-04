#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/camera.h>
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/shaders/sampler_shader.h>
#include <microgl/samplers/texture.h>
#include "data/model_3d_cube.h"

using namespace microgl::shading;
#define TEST_ITERATIONS 0
#define W 640*1
#define H 640*1
#define TTT 1
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;
Resources resources{};

using namespace microgl::shading;
using index_t = unsigned int;
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;

Canvas24 * canvas;
Texture24 tex_1, tex_2;

void loop();
void init_sdl(int width, int height);

float t = 0;

// bitmap for mapping
Bitmap24 *bmp_uv;

float z=0.0;
template <typename number>
void test_shader_texture_3d(const model_3d<number> & object) {

    using vertex = vec3<number>;
    using camera = microgl::camera<number>;
    using mat4 = matrix_4x4<number>;
    using math = microgl::math;
    using vertex_attribute= sampler_shader_vertex_attribute<number>;

//    z-=0.0004;
//    z-=0.0425;
    z-=1.5;
//    z-=3;

    // setup mvp matrix
    mat4 model = mat4::transform({ math::deg_to_rad(z/2), math::deg_to_rad(z/2), math::deg_to_rad(z/2)},
                                 {0,0,-z/10.0f}, {10,10,10});
//    mat4 view = camera::lookAt({0, 0, 30}, {0,0, 0}, {0,1,0});
    mat4 view = camera::angleAt({0, 0, 300}, 0, math::deg_to_rad(-0),0);
    mat4 projection = camera::perspective(math::deg_to_rad(60), canvas->width(), canvas->height(), 1, 500);
//    mat4 projection= camera::orthographic(-canvas->width()/2, canvas->width()/2, -canvas->height()/2, canvas->height()/2, 1, 100);
    mat4 mvp= projection*view*model;

    // setup shader
    sampler_shader<number, Texture24> shader;
    shader.matrix= mvp;
    shader.sampler= &tex_1;

    // model to vertex buffers
    dynamic_array<vertex_attribute> vertex_buffer{object.vertices.size()};
    for (unsigned ix = 0; ix < object.vertices.size(); ++ix) {
        vertex_attribute v{};
        v.point= object.vertices[ix];
        v.uv= object.uvs[ix];
        vertex_buffer.push_back(v);
    }
//std::cout << z<<std::endl;
    // draw
    canvas->drawTriangles<blendmode::Normal, porterduff::None<>, false, true>(
            shader,
            W, H,
            vertex_buffer.data(),
            object.indices.data(),
            object.indices.size(),
            object.type,
            triangles::face_culling::ccw);
}

void render() {
    canvas->clear({255,255,255,255});

//    test_shader_color_2d<float>();
//    test_shader_color_2d<Q<10>>();
    test_shader_texture_3d<float>(cube_3d<float>);
//    test_shader_texture_3d<double>(cube_3d<double>);
//    test_shader_texture_3d<Q<16>>(cube_3d<Q<16>>);
//    test_shader_texture_3d<Q<10>>(cube_3d<Q<10>>);
//    test_shader_texture_3d<Q<5>>(cube_3d<Q<5>>);

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
    auto bmp_uv_U8 = new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_1.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
    canvas = new Canvas24(width, height);
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
