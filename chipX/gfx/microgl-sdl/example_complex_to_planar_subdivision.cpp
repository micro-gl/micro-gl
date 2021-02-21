#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/tesselation/planarize_division.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

using index_t = unsigned int;
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;

template <typename number>
dynamic_array<vec2<number>> box(float left, float top, float right, float bottom, bool ccw=false) {
    if(!ccw)
        return {
                {left,top},
                {right,top},
                {right,bottom},
                {left,bottom},
        };

    return{
            {left,top},
            {left,bottom},
            {right,bottom},
            {right,top},
    };
};

template <typename number>
chunker<vec2<number>> poly_inter_star() {
    chunker<vec2<number>> A;

    A.push_back_and_cut({
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    return A;
}


template <typename number>
chunker<vec2<number>> poly_inter_star_2() {
    chunker<vec2<number>> A;

    A.push_back_and_cut({
                                {150, 150},
                                {450,150},
                                {200,450},
                                {300,50},
                                {400,450},
                        });

    A.push_back_and_cut({
                                {150/2, 150/2},
                                {450/2,150/2},
                                {200/2,450/2},
                                {300/2,50/2},
                                {400/2,450/2},
                        });

    A.push_back_and_cut({
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
chunker<vec2<number>> box_1() {
    chunker<vec2<number>> A;

//    A.push_back_and_cut({
//                                {20,20},
//                                {400,20},
//                                {200,400},
//                        });

    A.push_back_and_cut(box<number>(50,50,300,300));
//    A.push_back_and_cut(box<number>(150,150,200,200));

    return A;
}


sampling::flat_color color_red {{255,0,255,255}};

template <typename number>
void render_polygon(chunker<vec2<number>> pieces, Canvas24 * canvas) {
    using index = unsigned int;
    using psd = microgl::tessellation::planarize_division<number>;

    canvas->clear({255, 255, 255, 255});

//    psd::compute_DEBUG(pieces, trapezes);
    dynamic_array<vec2<number>> trapezes;
    dynamic_array<vec2<number>> vertices;
    dynamic_array<index> indices;
    dynamic_array<triangles::boundary_info> boundary;
    triangles::indices type;
    psd::compute(pieces,
                 tessellation::fill_rule::even_odd,
                 tessellation::tess_quality::better,
                 vertices, type, indices,
                 &boundary, &trapezes);

    canvas->drawTriangles<blendmode::Normal, porterduff::FastSourceOverOnOpaque, true>(
            color_red,
            matrix_3x3<number>::identity(),
            vertices.data(),
            (vec2<number> *)nullptr,
            indices.data(),
            boundary.data(),
            indices.size(),
            type,
            50);

//        return;

    canvas->drawTrianglesWireframe({0,0,0,255},
            matrix_3x3<number>::identity(),
            vertices.data(),
            indices.data(),
            indices.size(),
            type,
            40);

    for (index ix = 0; ix < trapezes.size(); ix+=4) {
        canvas->drawWuLinePath({0,0,0,255},
                &trapezes[ix], 4, true);
    }

}

int main() {
    auto * canvas = new Canvas24(W, H);

    auto render = [&]() -> void {

//        render_polygon<float>(box_1<float>(), canvas);
//        render_polygon<float>(poly_inter_star<float>(), canvas);
        render_polygon<float>(poly_inter_star_2<float>(), canvas);
//
//        render_polygon<double>(poly_inter_star_2<double>(), canvas);
//        render_polygon<float>(poly_inter_star_2<float>(), canvas);
//        render_polygon<Q<0>>(poly_inter_star_2<Q<0>>(), canvas);
//
//        render_polygon<Q<0>>(poly_inter_star<Q<0>>(), canvas);
//        render_polygon<Q<10>>(poly_inter_star<Q<10>>(), canvas);
//
//        render_polygon<Q<1>>(poly_inter_star_2<Q<1>>(), canvas);
//        render_polygon<Q<2>>(poly_inter_star_2<Q<2>>(), canvas);
//        render_polygon<Q<3>>(poly_inter_star_2<Q<3>>(), canvas);
//        render_polygon<Q<4>>(poly_inter_star_2<Q<4>>(), canvas);
//        render_polygon<Q<8>>(poly_inter_star_2<Q<8>>(), canvas);
//        render_polygon<Q<12>>(poly_inter_star_2<Q<12>>(), canvas);
//        render_polygon<Q<15>>(poly_inter_star_2<Q<15>>(), canvas);
//        render_polygon<Q<16>>(poly_inter_star_2<Q<16>>(), canvas);
//        render_polygon<Q<17>>(poly_inter_star_2<Q<17>>(), canvas);
//        render_polygon<Q<18>>(poly_inter_star_2<Q<18>>(), canvas);
    };

    example_run(canvas,
                TEST_ITERATIONS,
                render);
}