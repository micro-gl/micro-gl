#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "src/Resources.h"
#include <microgl/Canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGBA8888_ARRAY.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGB8.h>
#include <microgl/samplers/texture.h>
#include <microgl/PaletteBitmap.h>
#include "data/packed_1.h"
#include "data/packed_2.h"
#include "data/packed_4.h"
#include "data/bitmap_1_palette_256_colors.h"

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
using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Bitmap8888_ARRAY= Bitmap<coder::RGBA8888_ARRAY>;
using Bitmap8= Bitmap<coder::RGB8>;
using PaletteBitmap_1= PaletteBitmap<1, coder::RGBA8888_ARRAY, true>;
using PaletteBitmap_2= PaletteBitmap<2, coder::RGBA8888_ARRAY, true>;
using PaletteBitmap_4= PaletteBitmap<4, coder::RGBA8888_ARRAY, true>;
using PaletteBitmap_8= PaletteBitmap<8, coder::RGBA8888_ARRAY, false>;
using Canvas24= Canvas<Bitmap24>;
using Texture24= sampling::texture<Bitmap24, sampling::texture_filter::NearestNeighboor>;
using TexPalette1= sampling::texture<PaletteBitmap_1>;
using TexPalette2= sampling::texture<PaletteBitmap_2>;
using TexPalette4= sampling::texture<PaletteBitmap_4>;
using TexPalette8= sampling::texture<PaletteBitmap_8, sampling::texture_filter::Bilinear>;
using font32= microgl::text::bitmap_font<Bitmap8888_ARRAY>;
Canvas24 * canvas;
Texture24 tex_uv;
TexPalette1 tex_1, tex_1_fill;
TexPalette2 tex_2, tex_2_fill;
TexPalette4 tex_4, tex_4_fill;
TexPalette8 tex_8, tex_8_fill;
font32 font;

void loop();
void init_sdl(int width, int height);
float t=0;

template <typename number>
void test_text() {
    text::text_format format;
//    font.lineHeight=19;
    format.leading=5;
    format.fontSize=-1;
//    format.horizontalAlign=text::hAlign::right;
    format.horizontalAlign=text::hAlign::left;
//    format.horizontalAlign=text::hAlign::center;
//    format.verticalAlign=text::vAlign::center;
//    format.verticalAlign=text::vAlign::bottom;
    format.verticalAlign=text::vAlign::top;
//    format.wordWrap=text::wordWrap ::normal;
    format.wordWrap=text::wordWrap::break_word;
//    canvas->drawText("hello hello helloaaaaaaaaaaaaaa hello hello hello ",
    canvas->drawText("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!№;%:?*()_+-=.,/|\"'@#$^&{}[]",
//    canvas->drawText("BBDDD dd",
                     font, {255, 255, 255, 255},format,
                     0, 0, 300, 300, true,
                     255);

}

template <typename number, typename TEX>
void test_texture(TEX & tex) {
        canvas->drawQuad<blendmode::Normal, porterduff::None<>, false>(
                tex,
                0, 0, tex.bitmap().width()>>0, tex.bitmap().height()>>0,
                255,
                0,0,
                1,1);
}

void render() {
    canvas->clear({73,84,101,255});
//    test_texture<float>(tex_1);
//    test_texture<float>(tex_1_fill);
//    test_texture<float>(tex_2);
//    test_texture<float>(tex_2_fill);
//    test_texture<float>(tex_4);
//    test_texture<float>(tex_4_fill);
    test_texture<float>(tex_8);
//    test_texture<float>(tex_4_fill);
//    test_text<float>();
}

int main() {
    init_sdl(W, H);
    loop();
}

void init_sdl(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    sdl_window = SDL_CreateWindow("SDL2 Pixel Drawing", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_ALLOW_HIGHDPI);
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888,
                                    SDL_TEXTUREACCESS_STATIC, width, height);
    auto img_2 = resources.loadImageFromCompressedPath("images/uv_256.png");
    auto bmp_uv_U8 = new Bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height);
    tex_uv.updateBitmap(bmp_uv_U8->convertToBitmap<coder::RGB888_PACKED_32>());
//    resources.loadFont<vec4<uint8_t>, coder::RGBA8888_ARRAY>("minecraft-20", font);
    resources.loadFont<Bitmap8888_ARRAY>("digital_7-20", font);
//    resources.loadFont<vec4<uint8_t>, coder::RGBA8888_ARRAY>("roboto-thin-28", font);
//    resources.loadFont<vec4<uint8_t>, coder::RGBA8888_ARRAY>("roboto-thin-14", font);
//    resources.loadFont<vec4<uint8_t>, coder::RGBA8888_ARRAY>("mont-med-16", font);
//    resources.loadFont<vec4<uint8_t>, coder::RGBA8888_ARRAY>("test_texture", font);
    canvas = new Canvas24(width, height);
//
//    auto * bitmap_packed_1 = new PaletteBitmap_1{font_map_1_bpp, 152, 128};
//    auto * bitmap_packed_1_fill = new PaletteBitmap_1{128, 128};
//    auto * bitmap_packed_2 = new PaletteBitmap_2{font_map_2_bpp, 148, 128};
//    auto * bitmap_packed_2_fill = new PaletteBitmap_2{128, 128};
//    auto * bitmap_packed_4 = new PaletteBitmap_4{font_map_4_bpp, 148, 128};
    auto * bitmap_packed_8 = new PaletteBitmap_8{bitmap_1_256_colors_data,
//                                                 reinterpret_cast<vec4<uint8_t> *>(bitmap_1_256_colors_palette),
                                                 (bitmap_1_256_colors_palette),
                                                 256, 256};
    tex_8.updateBitmap(bitmap_packed_8);

//    auto * bitmap_packed_4_fill = new PaletteBitmap_4{128, 128};
//    bitmap_packed_1_fill->fill(1); bitmap_packed_1_fill->writeAt(2,2,0); bitmap_packed_1_fill->writeAt(4,4,0);
//    bitmap_packed_2_fill->fill(3); bitmap_packed_2_fill->writeAt(2,2,0); bitmap_packed_2_fill->writeAt(4,4,2);
//    bitmap_packed_4_fill->fill(15); bitmap_packed_4_fill->writeAt(2,2,0); bitmap_packed_4_fill->writeAt(4,4,0);
//    tex_1.updateBitmap(bitmap_packed_1);
//    tex_1_fill.updateBitmap(bitmap_packed_1_fill);
//    tex_2.updateBitmap(bitmap_packed_2);
//    tex_2_fill.updateBitmap(bitmap_packed_2_fill);
//    tex_4.updateBitmap(bitmap_packed_4);
//    tex_4_fill.updateBitmap(bitmap_packed_4_fill);
}

int render_test(int N) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        render();
    auto end = std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return int_ms.count();}

void loop() {
    bool quit = false;
    SDL_Event event;

    // 100 Quads
    int ms = render_test(TEST_ITERATIONS);
    std::cout << ms << std::endl;

    while (!quit)
    {
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                if( event.key.keysym.sym == SDLK_ESCAPE )
                    quit = true;
                break;
        }
//
        render();

        SDL_UpdateTexture(sdl_texture, nullptr, canvas->pixels(),
                canvas->width() * canvas->sizeofPixel());
        SDL_RenderClear(sdl_renderer);
        SDL_Rect dst{0,0, W, H};
        SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
//        SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, &dst);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
