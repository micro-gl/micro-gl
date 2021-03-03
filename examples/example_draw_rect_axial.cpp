#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;

int main() {

    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    Resources resources{};
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto * canvas = new Canvas24(W, H);;
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    using number = float;
    int index = 0, ix=0;

    auto render = [&]() -> void {
//        index += coder::convert_channel_correct<5, 8>(ix++%255);
//        for (int ix = 0; ix < 1000; ++ix) {
//        }
        canvas->clear({255,255,255,255});
        canvas->drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true, number>(
                tex_uv,
                0, 0, 256, 256);
    };

    auto res = color::convert_channel_correct<10, 5>(1023);
    std::cout<< int(res) << std::endl;


    example_run(canvas, render);


}
