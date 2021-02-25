#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGBA8888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

// todo:: add font color
int main() {
    using index_t = unsigned int;
    using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
    using Bitmap32_ARRAY= Bitmap<coder::RGBA8888_ARRAY>;
    using Bitmap32_PACKED= Bitmap<coder::RGBA8888_PACKED_32>;
    using Canvas24= Canvas<Bitmap24>;
    using font32= microgl::text::bitmap_font<Bitmap32_ARRAY>;

    Resources resources{};

    Canvas24 * canvas = new Canvas24(W, H);;
    font32 font;

//    font = resources.loadFont<Bitmap24_ARRAY>("minecraft-20");
    font = resources.loadFont<Bitmap32_ARRAY>("digital_7-20");
//    font = resources.loadFont<Bitmap24_ARRAY>("roboto-thin-28");
//    font = resources.loadFont<Bitmap24_ARRAY>("roboto-thin-14");
//    font = resources.loadFont<Bitmap24_ARRAY>("mont-med-16");
//    font = resources.loadFont<Bitmap24_ARRAY>("test");

    auto render = [&]() -> void {
//        font.offsetX=5;
//        font.offsetY=5;
//        font.padding=5;
        text::text_format format;
//    font.lineHeight=19;
        format.leading=5;
        format.fontSize=-1;
//    format.horizontalAlign=text::hAlign::right;
        format.horizontalAlign=text::hAlign::left;
//    format.horizontalAlign=text::hAlign::center;
//    format.verticalAlign=text::vAlign::center;
//    format.verticalAlign=text::vAlign::bottom;
        format.verticalAlign=text::vAlign::top;
//    format.wordWrap=text::wordWrap ::normal;
        format.wordWrap=text::wordWrap::break_word;

        canvas->clear({73,84,101,255});
        canvas->drawText("hello hello helloaaaaaaaaaaaaaa hello hello hello ",
//        canvas->drawText("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!№;%:?*()_+-=.,/|\"'@#$^&{}[]",
//    canvas->drawText("BBDDD dd",
                         font,
                         {255, 0, 255, 255},
                         format,
                         0, 0, 300, 300,
                         true,
                         255);

    };

    example_run(canvas,
                render);
}
