#include "src/Resources.h"
#include "src/example.h"
#include <microgl/camera.h>
#include <microgl/z_buffer.h>
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/shaders/sampler_shader.h>
#include <microgl/samplers/texture.h>
#include "data/model_3d_cube.h"

#define TEST_ITERATIONS 1
#define W 640
#define H 480

int main() {
    using number = float;
//    using number = Q<5>;
//    using number = Q<10>;
//    using number = Q<15>;
//    using number = Q<16>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    Canvas24 canvas(W, H);
    Resources resources{};

    auto img = resources.loadImageFromCompressedPath("images/uv_256.png");
    Texture24 tex{new bitmap<coder::RGB888_ARRAY>(img.data, img.width, img.height)};
    z_buffer<12> depth_buffer(canvas.width(), canvas.height());

    float t = -0.0;
    constexpr bool enable_z_buffer = true;

    auto test_shader_texture_3d = [&](const model_3d<number> & object) {

        using vertex = vec3<number>;
        using camera = microgl::camera;
        using mat4 = matrix_4x4<number>;
        using math = microgl::math;
        using Shader = sampler_shader<number, Texture24>;
        using vertex_attributes = Shader::vertex_attributes;

//        t-=0.0425;
        t-=0.1425;

        // setup mvp matrix
        number radians = math::deg_to_rad(t / 2);
        vertex rotation = {radians, radians, radians};
        vertex translation = {-5,0, -t/10.f};
        vertex scale = {10,10,10};

        mat4 model_1 = mat4::transform(rotation, translation, scale);
        mat4 model_2 = mat4::transform(rotation*2, translation + vertex{10,0,0}, scale);
        mat4 view = camera::lookAt<number>({0, 0, 70}, {0,0, 0}, {0,1,0});
//        mat4 view = camera::angleAt<number>({0, 0, 70+ 0 / t}, 0,
//                                            math::deg_to_rad(0), 0);
        mat4 projection = camera::perspective<number>(math::deg_to_rad(60),
                                                      W, H, 10.f, 100);
//        mat4 projection= camera::orthographic<number>(-canvas->width()/2, canvas->width()/2,
//                                              -canvas->height()/2, canvas->height()/2, 1, 500);
        mat4 mvp_1= projection*view*model_1;
        mat4 mvp_2= projection*view*model_2;

        // setup shader
        Shader shader;
        shader.matrix= mvp_1;
        shader.sampler= &tex;

        // model to vertex buffers
        dynamic_array<vertex_attributes> vertex_buffer{object.vertices.size()};
        for (unsigned ix = 0; ix < object.vertices.size(); ++ix) {
            vertex_attributes v{};
            v.point= object.vertices[ix];
            v.uv= object.uvs[ix];
            vertex_buffer.push_back(v);
        }

        canvas.clear({255,255,255,255});
        depth_buffer.clear();
        // draw model_1
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, true, true, enable_z_buffer>(
                shader,
                canvas.width(), canvas.height(),
                vertex_buffer.data(),
                object.indices.data(),
                object.indices.size(),
                object.type,
                triangles::face_culling::ccw,
                &depth_buffer);
//            (z_buffer<0> *)nullptr);

        // draw model_2
        shader.matrix= mvp_2;
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, true, true, enable_z_buffer>(
                shader,
                canvas.width(), canvas.height(),
                vertex_buffer.data(),
                object.indices.data(),
                object.indices.size(),
                object.type,
                triangles::face_culling::ccw,
                &depth_buffer);

    };

    auto render = [&]() {
        static auto model = cube_3d<number>;

        test_shader_texture_3d(model);
    };

    example_run(&canvas, render);
}

