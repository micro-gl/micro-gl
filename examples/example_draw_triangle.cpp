#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/flat_color.h>

#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
float t=0;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    using number = Q<12>;
//    using number = float;

    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_512.png");
    Canvas24 canvas(W, H);;
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    flat_color<> color_sampler{{255,122,0}};

    auto render_flat = [&](void*, void *, void*) -> void {
        canvas.clear({255,255,255,255});
        canvas.drawTriangle<blendmode::Normal, porterduff::None<true>, false, number, number>(
                color_sampler,
                10.0f, 10.0f, 0.0f, 0.0f,
                400.0f, 10.0f, 1.0f, 0.0f,
                400.0f, 400.0f, 1.0f, 1.0f,
                255);
    };

    auto render_tex = [&](void*, void *, void*) -> void {
        t+=0.01f;
        canvas.clear({255,255,255,255});
        canvas.drawTriangle<blendmode::Normal, porterduff::None<>, false, number, number>(
                tex_uv,
                10.0   ,10.0, 0.0, 1.0,
                250.0+0,10.0, 1.0, 1.0,
                250.0+0,250.0, 1.0, 0.0,
                255);
    };

    auto render_tex_aa = [&](void*, void *, void*) -> void {
//        t+=0.01f;
        canvas.clear({255,255,255,255});
        canvas.drawTriangle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number, number>(
                tex_uv,
                10.0,10.0, 0.0, 1.0,
                500.0,10.0, 1.0, 1.0,
                500.0,500.0, 1.0, 0.0,
                255);
    };

//    example_run(&canvas, render_flat);
    example_run(&canvas, render_tex);
//    example_run(&canvas, render_tex_aa);
}
