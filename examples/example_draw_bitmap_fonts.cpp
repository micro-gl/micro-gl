#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_PACKED_32.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Bitmap32_ARRAY= bitmap<coder::RGBA8888_ARRAY>;
    using Bitmap32_PACKED= bitmap<coder::RGBA8888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using font32= microgl::text::bitmap_font<Bitmap32_ARRAY>;

    Canvas24 canvas(W, H);
    font32 font;

//        font = Resources::loadFont<Bitmap32_ARRAY>("minecraft-20");
//    font = Resources::loadFont<Bitmap32_ARRAY>("digital_7-20");
    font = Resources::loadFont<Bitmap32_ARRAY>("roboto-thin-28");
//    font = Resources::loadFont<Bitmap32_ARRAY>("roboto-thin-14");
//    font = Resources::loadFont<Bitmap32_ARRAY>("mont-med-16");
//    font = Resources::loadFont<Bitmap32_ARRAY>("test");

    auto render = [&](void*, void*, void*) -> void {
//        font.offsetX=5;
//        font.offsetY=5;
        font.padding=5;
        text::text_format format;
//    font.lineHeight=19;
        format.letterSpacing=0;
        format.leading=5;
//        format.fontSize=40;
//    format.horizontalAlign=text::hAlign::right;
        format.horizontalAlign=text::hAlign::left;
//    format.horizontalAlign=text::hAlign::center;
//    format.verticalAlign=text::vAlign::center;
//    format.verticalAlign=text::vAlign::bottom;
        format.verticalAlign=text::vAlign::top;
//    format.wordWrap=text::wordWrap ::normal;
        format.wordWrap=text::wordWrap::break_word;

        canvas.clear({73,84,101,255});
        canvas.drawText<true, true, true>("Welcome to micro{gl} Welcome to micro{gl} Welcome to "
                         "micro{gl} Welcome to micro{gl} Welcome to micro{gl} ",
//        canvas.drawText("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!№;%:?*()_+-=.,/|\"'@#$^&{}[]",
//        canvas.drawText("aaaaa",
                         font,
                         {255, 255, 255, 255},
                         format,
                         0, 0, 300, 300,
                         255);

    };

    example_run(&canvas,
                render);
}
