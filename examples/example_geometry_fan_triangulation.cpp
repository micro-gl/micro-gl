#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/samplers/flat_color.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/fan_triangulation.h>
#include "microgl/tesselation/static_array.h"
#include <vector>

#define W 640*1
#define H 480*1


template<typename item_type>
using static_arr = static_array<item_type, 100>;

template<typename item_type>
//using container = static_arr<item_type>;
using container = dynamic_array<item_type>;
//using container = std::vector<item_type>;

template <typename number>
container<vec2<number>> poly_diamond() {
    using il = std::initializer_list<vec2<number>>;
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

    using Canvas24 = canvas<bitmap<coder::RGB888_PACKED_32>>;
    Canvas24 canvas(W, H);
    sampling::flat_color<> color_red{{255, 0, 0, 255}};

    auto render_polygon = [&](const dynamic_array<vec2<number>> &polygon) {
        using index = unsigned int;

        container<index> indices;
        container<boundary_info> boundary_buffer;

        using fan = microtess::fan_triangulation<number, container<index>, container<boundary_info>>;

        canvas.clear({255, 255, 255, 255});

        auto type = microtess::triangles::indices::TRIANGLES_FAN_WITH_BOUNDARY;

        fan::compute(
                polygon.data(),
                polygon.size(),
                indices,
                &boundary_buffer,
                type
        );

        // draw triangles batch
        canvas.drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
                color_red,
                matrix_3x3<number>::identity(),
                polygon.data(),
                (vec2<number> *) nullptr,
                indices.data(),
                boundary_buffer.data(),
                indices.size(),
                type,
                120);

        // draw triangulation
        canvas.drawTrianglesWireframe(
                {0, 0, 0, 255},
                matrix_3x3<number>::identity(),
                polygon.data(),
                indices.data(),
                indices.size(),
                type,
                255);
    };

    auto render = [&](void*, void*, void*) -> void {
        render_polygon(poly_diamond<number>());
    };

    example_run(&canvas, render);
}
