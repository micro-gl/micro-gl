#include <iostream>
#include <chrono>
#include <SDL.h>
#include "src/Resources.h"
#include <microgl/canvas.h>
#include <microgl/camera.h>
#include <microgl/z_buffer.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/shaders/sampler_shader.h>
#include "data/model_3d_cube.h"

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1
SDL_Window * window;
SDL_Renderer * renderer;
Resources resources{};

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
using Canvas24= canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::Bilinear>;
Texture24 tex_uv;
Canvas24 * canva;
sampling::flat_color<> color_grey{{0,0,122,255}};
void loop();
void init_sdl(int width, int height);
float t=0;

/*
 * NOTE:
 * SDL_RenderCopy is very slow and it's performance is for some reason not linear,
 * so don't mind performance in these blocks examples
 */
float z=0;

template <typename number, typename z_buffer_type>
void render_block(int block_x, int block_y, Bitmap24 *bmp, z_buffer_type * depth_buffer, const model_3d<number> & object) {
    using l64= long long;
    using vertex = vec3<number>;
    using camera = microgl::camera;
    using mat4 = matrix_4x4<number>;
    using namespace microgl::math;
    using Shader= sampler_shader<number, Texture24>;
    using vertex_attributes= vertex_attributes<Shader>;

//    z-=0.0004;
    z-=0.425;

    // setup mvp matrix
    mat4 model_1 = mat4::transform({ math::deg_to_rad(z/2), math::deg_to_rad(z/2), math::deg_to_rad(z/2)},
                                   {-5,0,0}, {10,10,10});
    mat4 model_2 = mat4::transform({ math::deg_to_rad(z/1), math::deg_to_rad(z/2), math::deg_to_rad(z/2)},
                                   {5,0,0}, {10,10,10});
//    mat4 view = camera::lookAt<number>({0, 0, 30}, {0,0, 0}, {0,1,0});
    mat4 view = camera::angleAt<number>({0, 0, 70}, 0, math::deg_to_rad(0.0f),0);
    mat4 projection = camera::perspective<number>(math::deg_to_rad(60.0f), canva->width(), canva->height(), 1, 500);
//    mat4 projection= camera::orthographic<number>(-canva->width()/2, canva->width()/2, -canva->height()/2, canva->height()/2, 1, 1000);
    mat4 mvp_1= projection*view*model_1;
    mat4 mvp_2= projection*view*model_2;

    // setup shader
   Shader shader;
    shader.matrix= mvp_1;
    shader.sampler= &tex_uv;

    // model to vertex buffers
    dynamic_array<vertex_attributes> vertex_buffer{object.vertices.size()};
    for (unsigned ix = 0; ix < object.vertices.size(); ++ix) {
        vertex_attributes v{};
        v.point= object.vertices[ix];
        v.uv= object.uvs[ix];
        vertex_buffer.push_back(v);
    }

    // clear z-buffer
    depth_buffer->clear();

    canva->updateCanvasWindow(block_x, block_y, bmp);
    // draw model_1
    canva->drawTriangles<blendmode::Normal, porterduff::None<>, false, true, true>(
            shader, W, H,
            vertex_buffer.data(),
            object.indices.data(),
            object.indices.size(),
            object.type,
            triangles::face_culling::ccw,
            depth_buffer);

//    return;
    // draw model_2
    shader.matrix= mvp_2;
    canva->drawTriangles<blendmode::Normal, porterduff::None<>, false, true, true>(
            shader, W, H,
            vertex_buffer.data(),
            object.indices.data(),
            object.indices.size(),
            object.type,
            triangles::face_culling::ccw,
            depth_buffer);
}


template <typename number>
void render_blocks() {
    auto model = cube_3d<number>;
    bool debug = 1;
    int block_size = W/2;//W/10;//2;//W/13;
    int count_blocks_horizontal = 1+((W-1)/block_size); // with integer ceil rounding
    int count_blocks_vertical = 1+((H-1)/block_size); // with integer ceil rounding
    auto * bitmap = new Bitmap24(block_size, block_size);
    canva = new Canvas24(bitmap);
    z_buffer<16> depth_buffer(canva->width(), canva->height());

    auto * sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                           SDL_TEXTUREACCESS_STREAMING, block_size, block_size);

    canva->updateClipRect(0, 0, W, H);
    SDL_RenderClear(renderer);
    for (int iy = 0; iy < block_size*count_blocks_vertical; iy+=block_size) {
        for (int ix = 0; ix < block_size*count_blocks_horizontal; ix+=block_size) {
            canva->updateCanvasWindow(ix, iy, bitmap);
            canva->clear({255,255,255,255});
            render_block<number>(ix, iy, bitmap, &depth_buffer, model);
            SDL_Rect rect_source {0, 0, block_size, block_size};
            SDL_Rect rect_dest {ix, iy, block_size-debug, block_size-debug};
            SDL_UpdateTexture(sdl_texture,
                              &rect_source,
                              &canva->pixels()[0],
                              (canva->width()) * canva->sizeofPixel());
            SDL_RenderCopy(renderer, sdl_texture, &rect_source, &rect_dest);
        }
    }

    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(sdl_texture);
    delete bitmap;
}

void render() {
    render_blocks<float>();
//    render_blocks<Q<16>>();
}

int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 pixel_ Drawing", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_512.png");
    auto bmp_uv_U8 = new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
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
        render();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

