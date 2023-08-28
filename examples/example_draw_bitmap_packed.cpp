#include "src/example.h"
#include "src/Resources.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/BPP_RGB.h>
#include <microgl/samplers/texture.h>
#include <microgl/bitmaps/packed_bitmap.h>
#include <microgl/bitmaps/bitmap.h>
#include "data/packed_1.h"
#include "data/packed_2.h"
#include "data/packed_4.h"

#define TEST_ITERATIONS 100
#define BITS 4 // 1, 2, 4 bits
#define W 640*1
#define H 640*1

using namespace microgl::sampling;

int main() {
    using index_t = unsigned int;
    using BitmapPacked= packed_bitmap<BITS, coder::BPP_RGB<BITS, 8,8,8>, true>;
    using Canvas24= canvas<bitmap<microgl::coder::RGB888_PACKED_32>>;
    using TexPacked= sampling::texture<BitmapPacked>;

    Canvas24 canvas(W, H);

#if (BITS==1)
    auto * bitmap_packed = new BitmapPacked{font_map_1_bpp, 152, 128};
    auto * bitmap_packed_fill = new BitmapPacked{128, 128};
    bitmap_packed_fill->fill(1); bitmap_packed_fill->writeAt(2,2,0); bitmap_packed_fill->writeAt(4,4,0);
#elif (BITS==2)
    auto * bitmap_packed = new BitmapPacked{font_map_2_bpp, 148, 128};
    auto * bitmap_packed_fill = new BitmapPacked{128, 128};
    bitmap_packed_fill->fill(3); bitmap_packed_fill->writeAt(2,2,0); bitmap_packed_fill->writeAt(4,4,2);
#elif (BITS==4)
    auto * bitmap_packed = new BitmapPacked{font_map_4_bpp, 148, 128};
    auto * bitmap_packed_fill = new BitmapPacked{128, 128};
    bitmap_packed_fill->fill(15); bitmap_packed_fill->writeAt(2,2,0); bitmap_packed_fill->writeAt(4,4,0);
#endif
    TexPacked texPacked{bitmap_packed}, texPackedFill{bitmap_packed_fill};

    bitmap_packed->pixelAt(0);

    auto render = [&](void*, void*, void*) -> void {

        canvas.clear(color_t{255,0,0});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false>(
                texPacked,
//                texPackedFill,
                0, 0,
                texPacked.bitmap().width()<<1, texPacked.bitmap().height()<<1,
                255,
                0,0,
                1,1);
    };

    example_run(&canvas,
                render);
}
