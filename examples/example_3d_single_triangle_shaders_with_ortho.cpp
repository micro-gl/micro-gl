#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/z_buffer.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/shaders/color_shader.h>
#include <microgl/shaders/flat_color_shader.h>
#include <microgl/shaders/sampler_shader.h>
#include <microgl/samplers/texture.h>
#include <microgl/camera.h>

using namespace microgl::shading;
#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

float t = 0;

int main() {
    using number = float;
//    using number = Q<12>;

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    Resources resources{};

    auto img = resources.loadImageFromCompressedPath("images/uv_256.png");

    auto * canvas = new Canvas24(W, H);
    Texture24 tex{new bitmap<coder::RGB888_ARRAY>(img.data, img.width, img.height)};

    auto test_shader_color_2d = [&]() {
        color_shader<number, 8,8,8,0> shader;
        shader.matrix= camera<number>::orthographic(0, W, 0, H, 0, 100);

        color_shader_vertex_attributes<number> v0, v1, v2;
        v0.point= {10.0,10.0, 0};  v0.color= {255,0,0,255};
        v1.point= {400.0,200.0, 0}; v1.color= {0,255,0,255};
        v2.point= {10.0,400.0, 0}; v2.color= {0,0,255,255};

        canvas->drawTriangle<blendmode::Normal, porterduff::None<>, false>(
                shader,W, H, v0, v1, v2, 255,
                triangles::face_culling::none,
                (z_buffer<0> *)nullptr);
    };

    auto test_shader_flat_color_2d = [&]() {
        flat_color_shader<number, 8,8,8,0> shader;
        color_t color{255,0,0,255};
        shader.matrix= camera<number>::orthographic(0, W, 0, H, 0, 100);
        shader.color= color;
        flat_color_shader_vertex_attributes<number> v0, v1, v2;
        v0.point= {10.0,10.0, 0};
        v1.point= {500.0,10.0, 0};
        v2.point= {500.0,500.0, 0};
        t+=0.0001;
        canvas->drawTriangle<blendmode::Normal, porterduff::None<>, false>(
                shader, W, H, v0, v1, v2, 255,
                triangles::face_culling::none,
                (z_buffer<0> *)nullptr);

    };

    auto test_shader_texture_2d = [&]() {

        sampler_shader<number, Texture24> shader;
        shader.matrix= camera<number>::orthographic(0, W, 0, H, 0, 10);
        shader.sampler= &tex;

        sampler_shader_vertex_attribute<number> v0{}, v1{}, v2{};
        v0.point= {10.0,10.0, 0};   v0.uv= {0.0f, 0.0f};
        v1.point= {500.0,10.0, 0};  v1.uv= {1.0f, 0.0f};
        v2.point= {500.0,500.0, 0}; v2.uv= {1.0f, 1.0f};
        t+=0.001f;
        canvas->drawTriangle<blendmode::Normal, porterduff::None<>, false>(
                shader, W, H, v0, v1, v2, 255,
                triangles::face_culling::none,
                (z_buffer<0> *)nullptr);

    };

    auto render = [&]() {
        canvas->clear({255,255,255,255});

        test_shader_color_2d();
//        test_shader_texture_2d();
//        test_shader_flat_color_2d();
    };

    example_run(canvas, render);
}
