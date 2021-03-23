#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/angular_linear_gradient.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
//    using number = float;
    using number = Q<12>;

    Canvas24 canvas(W, H);;
    angular_linear_gradient<number, 3, Canvas24::rgba> gradient{45};

    auto render = [&]() -> void {
        static number t = 0;
        t+=number(0.01);

        gradient.setAngle(t);
        gradient.addStop(0.0f, {255,0,0});
        gradient.addStop(0.5f, {0,255,0});
        gradient.addStop(1.f, {0,0,255});

        canvas.clear({255,255,255,255});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false, number>(gradient, 0, 0, 400, 400);
    };

    example_run(&canvas, render);
}

