#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>

#define W 640*1
#define H 480*1

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
//        canvas.clear(intensity<Q<10>>{1.0, 0.50, 0.0,1.0});
        canvas.clear(color_t{255,0,0});
    };

    example_run(&canvas, render);
}
