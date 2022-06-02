#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/samplers/flat_color.h>

#include <micro-tess/planarize_division.h>
#include <micro-tess/static_array.h>
#include <micro-tess/dynamic_array.h>
#include <micro-tess/chunker.h>
#include <micro-tess/std_rebind_allocator.h>
#include <vector>

template<typename item>
using fixed_static_array = static_array<item, 10000>;

// containers for collections of points (polygons)
template<typename number>
using chunker_t = allocator_aware_chunker<vertex2<number>, dynamic_array>;
//using chunker_t = allocator_aware_chunker<vertex2<number>, std::vector>;
//using chunker_t = non_allocator_aware_chunker<vertex2<number>, fixed_static_array>;

// containers templates for output containers
template<typename item>
using container = dynamic_array<item>;
//using container = std::vector<item>;
//using container = static_array<item, 10000>;

template <typename number>
container<vertex2<number>> box(float left, float top, float right,
                               float bottom, bool ccw=false) {
    using il = std::initializer_list<vertex2<number>>;

    if(!ccw)
        return il{
                {left,top},
                {right,top},
                {right,bottom},
                {left,bottom},
        };

    return il{
            {left,top},
            {left,bottom},
            {right,bottom},
            {right,top},
    };
};

template <typename number>
chunker_t<number> poly_inter_star() {
    using il = std::initializer_list<vertex2<number>>;
    chunker_t<number> A;

    A.push_back_and_cut(il{
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    return A;
}

template <typename number>
chunker_t<number> poly_inter_star_2() {
    using il = std::initializer_list<vertex2<number>>;
    chunker_t<number> A;

    A.push_back_and_cut(il{
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    A.push_back_and_cut(il{
                                {150/2, 150/2},
                                {450/2,150/2},
                                {200/2,450/2},
                                {300/2,50/2},
                                {400/2,450/2},
                        });

    A.push_back_and_cut(il{
                                {150/10, 150/10},
                                {450/10,150/10},
                                {200/10,450/10},
                                {300/10,50/10},
                                {400/10,450/10},
                        });

    A.push_back_and_cut(box<number>(50,50,300,300, false));
    A.push_back_and_cut(box<number>(50,250,600,300, true));
    A.push_back_and_cut(box<number>(50,450,100,500, true));

    return A;
}

template <typename number>
chunker_t<number> box_1() {
    chunker_t<number> A;
    A.push_back_and_cut(box<number>(50,50,300,300));
    return A;
}

int main() {
    using number = float;
//    using number = double;
//    using number = Q<2>;
//    using number = Q<4>;
//    using number = Q<8>;
//    using number = Q<12>;
//    using number = Q<15>;
//    using number = Q<16>;

    // microgl drawing setup START
    using Canvas24= canvas<bitmap<RGB888_PACKED_32>>;
    sampling::flat_color<> color_red {{255, 0, 255, 255}};
    Canvas24 canvas(640, 480);
    // microgl drawing setup END

    auto render_polygon = [&](const chunker_t<number> & pieces) {
        using index = unsigned int;

        // Algorithm START
        // output tess vertices
        container<vertex2<number>> output_vertices;
        // output tess indices
        container<index> output_indices;
        // output optional approximate trapezes
        container<vertex2<number>> output_optional_approximate_trapezes;
        // output optional boundary info
        container<microtess::triangles::boundary_info> output_boundary;
        // output triangles indices type
        microtess::triangles::indices output_triangles_type;
        // optional computation allocator
        microtess::std_rebind_allocator<> allocator;
        // define algorithm
        using psd = microtess::planarize_division<
                            number,
                            decltype(output_vertices),
                            decltype(output_indices),
                            decltype(output_boundary),
                            decltype(allocator),
                            true, 200>;
        // compute algorithm
        psd::template compute<decltype(pieces)>(pieces,
                                microtess::fill_rule::even_odd,
                                microtess::tess_quality::better,
                                output_vertices,
                                output_triangles_type,
                                output_indices,
                                &output_boundary,
                                &output_optional_approximate_trapezes,
                                allocator);
        // Algorithm END

        canvas.clear({255, 255, 255, 255});
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, false>(
                color_red,
                matrix_3x3<number>::identity(),
                output_vertices.data(),
                (vertex2<number> *)nullptr,
                output_indices.data(),
                output_boundary.data(),
                output_indices.size(),
                output_triangles_type,
                255);

//        return;
        canvas.drawTrianglesWireframe({0,0,0,255},
                                      matrix_3x3<number>::identity(),
                                      output_vertices.data(),
                                      output_indices.data(),
                                      output_indices.size(),
                                      output_triangles_type,
                                      40);

        for (index ix = 0; ix < output_optional_approximate_trapezes.size(); ix+=4) {
            canvas.drawWuLinePath({0,0,0,255},
                                  &output_optional_approximate_trapezes[ix], 4, true);
        }

    };

    auto render = [&](void*, void*, void*) -> void {
//        static auto polygons = poly_inter_star<number>();
        static auto polygons = poly_inter_star_2<number>();
//        static auto polygons = box_1<number>();

        render_polygon(polygons);
    };

    example_run(&canvas, render);
}
