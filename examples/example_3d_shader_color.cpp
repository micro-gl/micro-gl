#include "src/example.h"
#include <microgl/camera.h>
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/z_buffer.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/shaders/color_shader.h>
#include "data/model_3d_cube.h"

#define W 640*1
#define H 640*1

int main() {
    using number = float;
//    using number = Q<5>;
//    using number = Q<10>;
//    using number = Q<15>;
//    using number = Q<16>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_64_BIT>;
    Canvas24 canvas(W, H);
    float t = -30.0;

    auto test_shader_texture_3d = [&](const model_3d<number> & object) {

        using vertex = vertex3<number>;
        using camera = microgl::camera;
        using mat4 = matrix_4x4<number>;
        using namespace microgl::math;
        using shader_ = color_shader<number, rgba_t<8,8,8,0>>;
        using vertex_attributes = shader_::vertex_attributes;

        t-=0.0425;

        // setup mvp matrix
        number radians = math::deg_to_rad(number{t} / 2);
        vertex rotation = {radians, radians, radians};
        vertex translation = {-5,0, 0};
        vertex scale = {10,10,10};
        mat4 model = mat4::transform(rotation, translation, scale);
        mat4 view = camera::lookAt<number>({0, 0, 100}, {0,0, 0}, {0,1,0});
//        mat4 view = camera::angleAt<number>({0, 0, 70}, 0,0, 0);
        mat4 projection = camera::perspective<number>(math::deg_to_rad(60.0f),
                                                      canvas.width(), canvas.height(), 20, 100);
//        mat4 projection= camera::orthographic<number>(-W/2, W/2,-H/2, H/2, 1, 500);
//        mat4 projection= camera::orthographic<number>(0, W,0, H, 1, 500);
        mat4 mvp= projection*view*model;

        // setup shader
        shader_ shader;
        shader.matrix= mvp;

        // model to vertex buffers
        dynamic_array<vertex_attributes> vertex_buffer{};
        vertex_buffer.reserve(object.vertices.size());

        for (unsigned ix = 0; ix < object.vertices.size(); ++ix) {
            shader_::vertex_attributes v{};
            channel_t r= (ix%3)==0 ? 255:0;
            channel_t g= (ix%3)==1 ? 255:0;
            channel_t b= (ix%3)==2 ? 255:0;
            v.point= object.vertices[ix];
            v.color={r,g,b,255};
            vertex_buffer.push_back(v);
        }

        // draw model_1
        canvas.clear({255,255,255,255});
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, true, true, false>(
                shader,
                canvas.width(), canvas.height(),
                vertex_buffer.data(),
                object.indices.data(),
                object.indices.size(),
                object.type,
                microtess::triangles::face_culling::ccw,
                (z_buffer<0> *)nullptr);
    };

    auto render = [&](void*, void*, void*) {
        static auto model = cube_3d<number>;

        test_shader_texture_3d(model);
    };

    example_run(&canvas, render);

    return 0;
}

