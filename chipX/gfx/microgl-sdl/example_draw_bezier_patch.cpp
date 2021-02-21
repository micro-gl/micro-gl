#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 1
#define W 640*1
#define H 640*1

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
sampling::flat_color color_grey{{222,222,222,255}};
Texture24 tex_uv;

template <typename number>
void test_bezier(Canvas24 *canvas, vec3<number>* mesh, unsigned U, unsigned V) {
    canvas->drawBezierPatch<blendmode::Normal, porterduff::None<>, false, false, number, number>(
//            color_grey,
            tex_uv,
            matrix_3x3<number>::identity(),
            mesh, U, V, 40, 40,
            0,1,1,0,
            255);
    delete [] mesh;
}

template <typename number>
vec3<number>* bi_cubic_1(){

    return new vec3<number>[4*4] {
                {1.0f, 0.0f},
                {170.66f, 0.0f},
                {341.333f, 0.0f},
                {512.0f, 0.0f},

                {1.0f,       170.66f},
                {293.44f,    162.78f},
                {746.68f,    144.65f},
                {512.0f,     170.66f},

                {1.0f,       341.33f},
                {383.12f,    327.69f},
                {1042.79f,   296.31f},
                {512.0f,     341.33f},

                {1.0f,       512.0f},
                {170.66f,    512.0f},
                {341.333f,   512.0f},
                {512.0f,     512.0f}
    };
}

int main() {
    Resources resources{};

    Canvas24 * canvas = new Canvas24(W, H);

    //    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_512.png");
    auto bmp_uv_U8 = new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
//        test_bezier<float>(canvas, bi_cubic_1<float>(), 4, 4);
//        test_bezier<double>(canvas, bi_cubic_1<double>(), 4, 4);
        test_bezier<Q<16>>(canvas, bi_cubic_1<Q<16>>(), 4, 4);
    };

    example_run(canvas,
                TEST_ITERATIONS,
                render);
}
