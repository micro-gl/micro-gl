#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>
#include <vector>
#include <microgl/static_array.h>

template<typename item_type>
using static_arr = static_array<item_type, 100>;

template<typename item_type>
//using container = static_arr<item_type>;
using container = dynamic_array<item_type>;
//using container = std::vector<item_type>;

#define W 640*1
#define H 480*1

float t = 0;

template <typename number>
container<vec2<number>> curve_cubic_1() {
    using il=std::initializer_list<vec2<number>>;
    using vertex=vec2<number>;
    return il{{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}};
}


int main() {
    using number = float;
//    using number = Q<12>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    Resources resources{};
    sampling::flat_color<> red{{255, 0, 0, 255}};

    Canvas24 canvas(W, H);

    const auto render_cubic_1 = [&](container<vec2<number>> curve) {
        static number t = 0;
        using curve_divider = microgl::tessellation::curve_divider<number, dynamic_array>;
        auto algo = microgl::tessellation::CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium;
        auto type = microgl::tessellation::CurveType::Cubic;
        container<vec2<number>> output{};

        t += number(0.08);
        curve[1].y -= t;
        curve_divider::compute(curve.data(), output, algo, type);

        canvas.clear({255,255,255,255});
        canvas.drawWuLinePath(
                {0,0,0,255},
                output.data(), output.size(),
                false);

        for (auto & p : output) {
            canvas.drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                    red, red, p.x, p.y, number{5}, number{0});
        }
    };

    const auto render = [&]() {

        render_cubic_1(curve_cubic_1<number>());
    };

    example_run(&canvas, render);
}