#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#include <micro-tess/curve_divider.h>
#include <micro-tess/static_array.h>
#include <micro-tess/dynamic_array.h>
#include <vector>

template<typename item_type>
//using container = static_array<item_type, 100>;
using container = dynamic_array<item_type>;
//using container = std::vector<item_type>;

#define W 640*1
#define H 480*1

template <typename number>
container<vertex2<number>> curve_cubic_1() {
    using il=std::initializer_list<vertex2<number>>;
    using vertex=vertex2<number>;
    return il{{5, H - 5}, {W/8, H/4}, {W/3, H/2}, {W/2, H/2}};
}

int main() {
//    using number = float;
    using number = Q<12>;

    // microgl drawing setup START
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    sampling::flat_color<> red{{255, 0, 0, 255}};
    Canvas24 canvas(W, H);
    // microgl drawing setup END

    const auto render_cubic_1 = [&](container<vertex2<number>> curve) {
        static number t = 0;
        t += number(0.08);
        curve[1].y -= t;

        // Algorithm START
        // output vertices
        container<vertex2<number>> output_vertices;
        // algorithm strategy
        auto algo_type = microtess::CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium;
        // the type of the curve
        auto curve_type = microtess::CurveType::Cubic;
        // define algorithm
        using curve_divider = microtess::curve_divider<number, decltype(output_vertices)>;
        // compute
        curve_divider::compute(curve.data(), output_vertices, algo_type, curve_type);
        // Algorithm END

        canvas.clear({255,255,255,255});
        canvas.drawWuLinePath(
                {0,0,0,255},
                output_vertices.data(), output_vertices.size(),
                false);

        for (auto & p : output_vertices)
            canvas.drawCircle<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                    red, red, p.x, p.y, number{5}, number{0});
    };

    auto render = [&](void*, void*, void*) -> void {
        render_cubic_1(curve_cubic_1<number>());
    };

    example_run(&canvas, render);
}