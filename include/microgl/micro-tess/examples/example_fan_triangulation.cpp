#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>

#include <micro-tess/fan_triangulation.h>
#include <micro-tess/static_array.h>
#include <micro-tess/dynamic_array.h>
#include <vector>

template<typename item_type>
using container = dynamic_array<item_type>;
//using container = static_array<item_type, 100>;
//using container = std::vector<item_type>;

template <typename number>
container<vertex2<number>> poly_diamond() {
    using il = std::initializer_list<vertex2<number>>;
    return il{
        {100,300},
        {300, 100},
        {400, 300},
        {300, 400}
    };
}

int main() {
    using number = float;
//    using number = Q<12>;

    // microgl drawing setup START
    using Canvas24 = canvas<bitmap<coder::RGB888_PACKED_32>>;
    Canvas24 canvas(640, 480);
    sampling::flat_color<> color_red{{255, 0, 0, 255}};
    // microgl drawing setup END

    auto render_polygon = [&](const container<vertex2<number>> &polygon) {

        // output indices container
        container<unsigned int> output_indices;
        // output boundary info container
        container<boundary_info> output_boundary_buffer;
        // output triangles indices type
        microtess::triangles::indices output_triangles_type;
        // define algorithm
        using fan = microtess::fan_triangulation<number, container<unsigned int>,
                container<boundary_info>>;
        // compute algorithm
        fan::compute(
                polygon.data(),
                polygon.size(),
                output_indices,
                &output_boundary_buffer,
                output_triangles_type
        );

        // draw triangles batch
        canvas.clear({255, 255, 255, 255});
        canvas.drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                matrix_3x3<number>::identity(),
                polygon.data(),
                (vertex2<number> *) nullptr,
                output_indices.data(),
                output_boundary_buffer.data(),
                output_indices.size(),
                output_triangles_type,
                120);

        // draw triangulation
        canvas.drawTrianglesWireframe(
                {0, 0, 0, 255},
                matrix_3x3<number>::identity(),
                polygon.data(),
                output_indices.data(),
                output_indices.size(),
                output_triangles_type,
                255);
    };

    auto render = [&](void*, void*, void*) -> void {
        render_polygon(poly_diamond<number>());
    };

    example_run(&canvas, render);
}
