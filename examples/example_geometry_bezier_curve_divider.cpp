#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/tesselation/curve_divider.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

float t = 0;

template <typename number>
dynamic_array<vec2<number>> curve_cubic_1() {
    using vertex=vec2<number>;
    return {{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}};
}


int main() {
    using number = float;
//    using number = Q<12>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    Resources resources{};
    sampling::flat_color<> red{{255, 0, 0, 255}};

    auto * canvas = new Canvas24 (W, H);

    const auto render_cubic_1 = [&](dynamic_array<vec2<number>> curve) {
        static number t = 0;
        using ear = microgl::tessellation::curve_divider<number>;
        auto algo = microgl::tessellation::CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium;
        auto type = microgl::tessellation::CurveType::Cubic;
        dynamic_array<vec2<number>> output{};

        t += number(0.08);
        curve[1].y -= t;
        ear::compute(curve.data(), output, algo, type);

        canvas->clear({255,255,255,255});
        canvas->drawWuLinePath(
                {0,0,0,255},
                output.data(), output.size(),
                false);

        for (int ix = 0; ix < output.size(); ++ix) {
            const auto & p = output[ix];
            canvas->drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                    red, red, p.x, p.y, number{5}, number{0});

        }
    };

    const auto render = [&]() {

        render_cubic_1(curve_cubic_1<number>());
    };

    example_run(canvas, render);
}