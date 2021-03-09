#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGBA_PACKED.h>
#include <microgl/pixel_coders/RGBA_PACKED_2.h>
#include <microgl/pixel_coders/RGBA_UNPACKED.h>
//#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_PACKED_32.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
float t=0;

int main() {
    using RGB888_PACKED_32 = coder::RGBA_PACKED<8,8,8,0>;
    using RGB888_PACKED_16 = coder::RGBA_PACKED<8,8,0,0>;
    using RGB888_ARRAY = coder::RGBA_UNPACKED<8,8,8,0>;
    using RGBA8888_ARRAY = coder::RGBA_UNPACKED<8,8,8,8>;
    using packed_1 = coder::RGBA_PACKED_2<5,6,5,8>;

    RGB888_PACKED_16::pixel aa;
    int bb = sizeof(RGB888_ARRAY::pixel);

    std::cout << bb << std::endl;

    using Canvas24= canvas<bitmap<RGB888_PACKED_32>, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
    using Texture24= sampling::texture<bitmap<RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    using Texture32= sampling::texture<bitmap<RGBA8888_ARRAY>>;
//    using number = Q<12>;
    using number = float;

    Resources resources{};
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto img_1 = resources.loadImageFromCompressedPath("images/a.png");
    auto * canvas = new Canvas24(W, H);;
    Texture24 tex_uv{new bitmap<RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    Texture32 tex_uv_32{new bitmap<RGBA8888_ARRAY>(img_1.data, img_1.width, img_1.height)};

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
                tex_uv,
//                tex_uv_32,
                0, 0, 300, 300);
    };

    example_run(canvas, render);
}
