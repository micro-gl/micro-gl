#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/bitmaps/PaletteBitmap.h>
#include "data/bitmap_1_palette_256_colors.h"
#include "data/bitmap_1_palette_16_colors.h"
#include "data/bitmap_1_palette_4_colors.h"
#include "data/bitmap_1_palette_2_colors.h"

#define PALETTE_SIZE 256 // 2, 4, 16, 256
#define W 640*1
#define H 640*1

int main() {
    using Bitmap24= bitmap<coder::RGB888_PACKED_32>;
    using Canvas24= canvas<Bitmap24>;
    using PaletteBitmap= PaletteBitmap<PALETTE_SIZE, coder::RGBA8888_ARRAY, false>;
    using TexPalette= sampling::texture<PaletteBitmap, sampling::texture_filter::NearestNeighboor>;

    Canvas24 canvas(W, H);;
    TexPalette tex;

#if (PALETTE_SIZE==2)
    auto * bitmap_packed = new PaletteBitmap{bitmap_1_2_colors_data, bitmap_1_2_colors_palette, 256, 256};
    tex.updateBitmap(bitmap_packed);
#elif (PALETTE_SIZE==4)
    auto * bitmap_packed = new PaletteBitmap{bitmap_1_4_colors_data, bitmap_1_4_colors_palette, 256, 256};
    tex.updateBitmap(bitmap_packed);
#elif (PALETTE_SIZE==16)
    auto * bitmap_packed = new PaletteBitmap{bitmap_1_16_colors_data, bitmap_1_16_colors_palette, 256, 256};
    tex.updateBitmap(bitmap_packed);
#elif (PALETTE_SIZE==256)
    auto * bitmap_packed = new PaletteBitmap{bitmap_1_256_colors_data, bitmap_1_256_colors_palette, 256, 256};
    tex.updateBitmap(bitmap_packed);
#endif

    auto render = [&]() -> void {

        canvas.clear(color_t{0,0,0});
        canvas.drawRect<blendmode::Normal, porterduff::None<>, false>(
                tex,
                0, 0, 400, 400, //tex.bitmap().width()>>0, tex.bitmap().height()>>0,
                255,
                0, 0,
                1, 1);

    };

    example_run(&canvas,
                render);
}
