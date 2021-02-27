#include "src/Resources.h"
#include "src/example.h"
#include <microgl/camera.h>
#include <microgl/Canvas.h>
#include <microgl/z_buffer.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/shaders/color_shader.h>
#include "data/model_3d_cube.h"

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

int main() {
    using number = float;
//    using number = Q<5>;
//    using number = Q<10>;
//    using number = Q<15>;
//    using number = Q<16>;

//    using Canvas24= Canvas<Bitmap<coder::RGB888_PACKED_32>>;
    using Canvas24= Canvas<Bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
    auto * canvas = new Canvas24(W, H);
    float t = -30.0;

    auto test_shader_texture_3d = [&](const model_3d<number> & object) {

        using vertex = vec3<number>;
        using camera = microgl::camera<number>;
        using mat4 = matrix_4x4<number>;
        using math = microgl::math;
        using vertex_attribute= color_shader_vertex_attributes<number>;

        t-=0.0425;

        // setup mvp matrix
        mat4 model_1 = mat4::transform({
                                               math::deg_to_rad(t / 2),
                                               math::deg_to_rad(t / 2),
                                               math::deg_to_rad(t / 2)},
                                       {-5,0, 0},
                                       {10,10,10});
//        mat4 view = camera::lookAt({0, 0, 30}, {0,0, 0}, {0,1,0});
        mat4 view = camera::angleAt({0, 0, 70+ 0 / t}, 0,
                                    math::deg_to_rad(0), 0);
        mat4 projection = camera::perspective(math::deg_to_rad(60),
                                              canvas->width(), canvas->height(), 20, 100);
//        mat4 projection= camera::orthographic(-canvas->width()/2, canvas->width()/2,
//                                              -canvas->height()/2, canvas->height()/2, 1, 500);
        mat4 mvp_1= projection*view*model_1;

        // setup shader
        color_shader<number,8,8,8,0> shader;
        shader.matrix= mvp_1;

        // model to vertex buffers
        dynamic_array<vertex_attribute> vertex_buffer{object.vertices.size()};
        for (unsigned ix = 0; ix < object.vertices.size(); ++ix) {
            vertex_attribute v{};
            channel r= (ix%3)==0 ? 255:0;
            channel g= (ix%3)==1 ? 255:0;
            channel b= (ix%3)==2 ? 255:0;
            v.point= object.vertices[ix];
            v.color={r,g,b,255};
            vertex_buffer.push_back(v);
        }

        // draw model_1
        canvas->drawTriangles<blendmode::Normal, porterduff::None<>, true, true, false>(
                shader,
                canvas->width(), canvas->height(),
                vertex_buffer.data(),
                object.indices.data(),
                object.indices.size(),
                object.type,
                triangles::face_culling::ccw,
                (z_buffer<0> *)nullptr);
    };

    auto render = [&]() {
        canvas->clear({255,255,255,255});
        canvas->updateClipRect(0,0,W,H);
        test_shader_texture_3d(cube_3d<number>);
    };

    example_run(canvas, render);
}

