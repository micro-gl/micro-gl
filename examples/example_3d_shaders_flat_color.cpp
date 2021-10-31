#include "src/example.h"
#include <microgl/camera.h>
#include <microgl/canvas.h>
#include <microgl/z_buffer.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/shaders/flat_color_shader.h>
#include <microgl/samplers/texture.h>
#include "data/model_3d_cube.h"

#define W 640*1
#define H 640*1

int main() {
    using number = float;
//    using number = Q<5>;
//    using number = Q<10>;
//    using number = Q<15>;
//    using number = Q<16>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
    Canvas24 canvas(W, H);

    float t = -30.0;

    auto test_shader_texture_3d = [&](const model_3d<number> & object) {

        using vertex = vec3<number>;
        using camera = microgl::camera;
        using mat4 = matrix_4x4<number>;
        using namespace microgl::math;
        using Shader = flat_color_shader<number, rgba_t<8,8,8,0>>;
        using vertex_attributes = Shader::vertex_attributes;

        t-=0.0425;

        // setup mvp matrix
        number radians = math::deg_to_rad(t / 2);
        vertex rotation = {radians, radians, radians};
        vertex translation = {-5,0, 0};
        vertex scale = {10,10,10};
        mat4 model = mat4::transform(rotation, translation, scale);
        mat4 view = camera::lookAt<number>({0, 0, 70}, {0,0, 0}, {0,1,0});
//        mat4 view = camera::angleAt<number>({0, 0, 70}, 0,0, 0);
        mat4 projection = camera::perspective<number>(math::deg_to_rad(60.0f),
                                              canvas.width(), canvas.height(), 20, 100);
//        mat4 projection= camera::orthographic<number>(-canvas->width()/2, canvas->width()/2,
//                                              -canvas->height()/2, canvas->height()/2, 1, 500);
        mat4 mvp_1= projection*view*model;

        // setup shader
        Shader shader;
        shader.matrix= mvp_1;
        shader.color= {0,0,0,255};

        // model to vertex buffers
        dynamic_array<vertex_attributes> vertex_buffer{};
        vertex_buffer.reserve(object.vertices.size());
        for (const auto & vert : object.vertices) {
            vertex_attributes v{};
            v.point= vert;
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
}
