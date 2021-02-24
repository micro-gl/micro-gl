#include "src/Resources.h"
#include "src/example.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1
SDL_Window * sdl_window;
SDL_Renderer * sdl_renderer;
SDL_Texture * sdl_texture;
Resources resources{};

using namespace microgl;
using namespace microgl::sampling;
using index_t = unsigned int;

int main() {
    using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= Canvas<Bitmap24, CANVAS_OPT_2d_raster_FORCE_32_BIT>;
    using Texture24= sampling::texture<Bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::NearestNeighboor>;

    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto * canvas = new Canvas24(W, H);;
    Texture24 tex_uv{new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    using number = float;
    float t =0;

    auto render = [&]() -> void {
        canvas->clear({255,255,255,255});
        t+=0.005;//-0.01;
        canvas->drawRect<blendmode::Normal, porterduff::None<>, false, number>(
                tex_uv,
                matrix_3x3<number>::rotation(microgl::math::deg_to_rad(5.0f+t), 128, 128),
//            matrix_3x3<number>::identity(),
                0, 0, 256<<0, 256<<0);
    };

    example_run(canvas, render);
}
