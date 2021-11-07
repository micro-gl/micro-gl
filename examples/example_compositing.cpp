#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_PACKED_32.h>
#include <microgl/porter_duff/Clear.h>
#include <microgl/porter_duff/Copy.h>
#include <microgl/porter_duff/Destination.h>
#include <microgl/porter_duff/DestinationAtop.h>
#include <microgl/porter_duff/DestinationIn.h>
#include <microgl/porter_duff/DestinationOut.h>
#include <microgl/porter_duff/DestinationOver.h>
#include <microgl/porter_duff/Lighter.h>
#include <microgl/porter_duff/None.h>
#include <microgl/porter_duff/Source.h>
#include <microgl/porter_duff/SourceAtop.h>
#include <microgl/porter_duff/SourceIn.h>
#include <microgl/porter_duff/SourceOut.h>
#include <microgl/porter_duff/SourceOver.h>
#include <microgl/porter_duff/XOR.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
float t=0;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>, CANVAS_OPT_32_BIT>;
    using Texture24= sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;
    using Texture32= sampling::texture<bitmap<coder::RGBA8888_ARRAY>>;
//    using number = Q<12>;
    using number = float;

    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_256.png");
    auto img_1 = Resources::loadImageFromCompressedPath("images/dog_32bit.png");
    Canvas24 canvas(W, H);;
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    Texture32 tex_uv_32{new bitmap<coder::RGBA8888_ARRAY>(img_1.data, img_1.width, img_1.height)};

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({255,255,255,255});
//        canvas.drawRect_internal<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::None<false>, false, number>(
        canvas.drawRect<blendmode::Normal, porterduff::SourceOver<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::XOR<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::Clear<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::Copy<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::Destination<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::DestinationAtop<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::DestinationIn<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::DestinationOut<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::DestinationOver<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::Lighter<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::None<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::Source<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::SourceAtop<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::SourceIn<true>, false, number>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::SourceOut<true>, false, number>(
//                tex_uv,
                tex_uv_32,
                0, 0, 300, 300,
                255);
    };

    example_run(&canvas, render, TEST_ITERATIONS);
}
