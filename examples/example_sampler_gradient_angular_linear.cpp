#include "src/example.h"
#define MICROGL_USE_STD_MATH
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/angular_linear_gradient.h>

#define W 640*1
#define H 640*1

using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    using number = float;
//    using number = Q<12>;

    Canvas24 canvas(W, H);
    angular_linear_gradient<number, 4, Canvas24::rgba, precision::high> gradient{0};

    auto render = [&](void*, void*, void*) -> void {
        static number t = 0;
        t+=number(0.1);

        gradient.setAngle(t);
        gradient.addStop(0.0f, {255,0,0});
        gradient.addStop(0.5f, {0,255,0});
        gradient.addStop(1.f, {0,0,255});

        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number, number>(
                gradient,
                0, 0, 400, 400);
    };

    example_run(&canvas, render);
}

