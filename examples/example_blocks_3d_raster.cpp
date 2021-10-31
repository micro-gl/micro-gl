#include <iostream>
#include "src/example.h"
#include "src/Resources.h"
#include "data/model_3d_cube.h"
#include <microgl/camera.h>
#include <microgl/canvas.h>
#include <microgl/z_buffer.h>
#include <microgl/shaders/sampler_shader.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1
#define BLOCK_SIZE W/4

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;

int main() {
    using number = float;
//    using number = <Q<16>;

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::Bilinear>;
    Texture24 tex_uv;
    sampling::flat_color<> color_grey{{122,122,122,255}};

    auto img_2 =Resources::loadImageFromCompressedPath("images/uv_512.png");
    auto bmp_uv_U8 = new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
    Canvas24 canvas(BLOCK_SIZE, BLOCK_SIZE);
    z_buffer<16> depth_buffer(canvas.width(), canvas.height());

    canvas.updateClipRect(0, 0, W, H);
    auto model = cube_3d<number>;

    auto render_block = [&](int block_x, int block_y) -> void {
        using l64= long long;
        using vertex = vec3<number>;
        using camera = microgl::camera;
        using mat4 = matrix_4x4<number>;
        using namespace microgl::math;
        using Shader= sampler_shader<number, Texture24>;
        using vertex_attributes= vertex_attributes<Shader>;
        static number z =0;
        //    z-=0.0004;
        z-=0.425;

        // setup mvp matrix
        mat4 model_1 = mat4::transform({ math::deg_to_rad(z/2), math::deg_to_rad(z/2), math::deg_to_rad(z/2)},
                                       {-5,0,0}, {10,10,10});
        mat4 model_2 = mat4::transform({ math::deg_to_rad(z/1), math::deg_to_rad(z/2), math::deg_to_rad(z/2)},
                                       {5,0,0}, {10,10,10});
        //    mat4 view = camera::lookAt<number>({0, 0, 30}, {0,0, 0}, {0,1,0});
        mat4 view = camera::angleAt<number>({0, 0, 70}, 0, math::deg_to_rad(0.0f),0);
        mat4 projection = camera::perspective<number>(math::deg_to_rad(60.0f), canvas.width(), canvas.height(), 1, 500);
        //    mat4 projection= camera::orthographic<number>(-canvas.width()/2, canvas.width()/2, -canvas.height()/2, canvas.height()/2, 1, 1000);
        mat4 mvp_1= projection*view*model_1;
        mat4 mvp_2= projection*view*model_2;

        // setup shader
        Shader shader;
        shader.matrix= mvp_1;
        shader.sampler= &tex_uv;

        // model to vertex buffers
        dynamic_array<vertex_attributes> vertex_buffer{model.vertices.size()};
        vertex_buffer.clear();
        for (unsigned ix = 0; ix < model.vertices.size(); ++ix) {
            vertex_attributes v{};
            v.point= model.vertices[ix];
            v.uv= model.uvs[ix];
            vertex_buffer.push_back(v);
        }

        // clear z-buffer
        depth_buffer.clear();

        canvas.updateCanvasWindow(block_x, block_y);
        // draw model_1
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, false, true, true>(
                shader, W, H,
                vertex_buffer.data(),
                model.indices.data(),
                model.indices.size(),
                model.type,
                microtess::triangles::face_culling::ccw,
                &depth_buffer);

        //    return;
        // draw model_2
        shader.matrix= mvp_2;
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, false, true, true>(
                shader, W, H,
                vertex_buffer.data(),
                model.indices.data(),
                model.indices.size(),
                model.type,
                microtess::triangles::face_culling::ccw,
                &depth_buffer);
    };

    auto render_blocks = [&](SDL_Renderer * renderer) -> void {
        bool debug = 1;
        int block_size = BLOCK_SIZE;//W/10;//2;//W/13;
        int count_blocks_horizontal = 1+((W-1)/block_size); // with integer ceil rounding
        int count_blocks_vertical = 1+((H-1)/block_size); // with integer ceil rounding

        auto * sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                                               SDL_TEXTUREACCESS_STREAMING, block_size, block_size);

        canvas.updateClipRect(0, 0, W, H);
        SDL_RenderClear(renderer);
        for (int iy = 0; iy < block_size*count_blocks_vertical; iy+=block_size) {
            for (int ix = 0; ix < block_size*count_blocks_horizontal; ix+=block_size) {
                canvas.updateCanvasWindow(ix, iy);
                canvas.clear({255,255,255,255});
                render_block(ix, iy);
                SDL_Rect rect_source {0, 0, block_size, block_size};
                SDL_Rect rect_dest {ix, iy, block_size-debug, block_size-debug};
                SDL_UpdateTexture(sdl_texture,
                                  &rect_source,
                                  &canvas.pixels()[0],
                                  (canvas.width()) * canvas.sizeofPixel());
                SDL_RenderCopy(renderer, sdl_texture, &rect_source, &rect_dest);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(sdl_texture);
    };

    auto render = [&](void*, SDL_Renderer * renderer, void*) -> void {
        render_blocks(renderer);
    };

    example_run(&canvas, W, H, render, 100, true);
}
