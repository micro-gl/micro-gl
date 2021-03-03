#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/color.h>
#include <microgl/intensity.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>

#define TEST_ITERATIONS 1000
#define W 640*1
#define H 480*1

int main() {
    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;

    auto * canvas = new Canvas24(W, H);

    auto render = [&]() -> void {
//        d.canvas->clear(intensity<Q<10>>{1.0, 0.50, 0.0,1.0});
        canvas->clear(color_t{255,0,0});
    };

    example_run(canvas,
                render);
}
