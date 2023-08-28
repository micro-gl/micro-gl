/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "rect.h"
#include "color.h"
#include "traits.h"
#include "masks.h"
#include "math.h"
#include "stdint.h"
#include "math/vertex2.h"
#include "math/matrix_3x3.h"
#include "pixel_coders/pixel_coder.h"
#include "porter_duff/FastSourceOverOnOpaque.h"
#include "porter_duff/DestinationIn.h"
#include "porter_duff/None.h"
#include "blend_modes/Normal.h"
#include "shaders/shader.h"
#include "samplers/texture.h"
#include "samplers/void_sampler.h"
#ifndef MICROGL_USE_EXTERNAL_MICRO_TESS
#include "micro-tess/include/micro-tess/triangles.h"
#include "micro-tess/include/micro-tess/polygons.h"
#include "micro-tess/include/micro-tess/path.h"
#include "micro-tess/include/micro-tess/monotone_polygon_triangulation.h"
#include "micro-tess/include/micro-tess/ear_clipping_triangulation.h"
#include "micro-tess/include/micro-tess/bezier_patch_tesselator.h"
#include "micro-tess/include/micro-tess/dynamic_array.h"
#else
#include <micro-tess/triangles.h>
#include <micro-tess/polygons.h>
#include <micro-tess/path.h>
#include <micro-tess/monotone_polygon_triangulation.h>
#include <micro-tess/ear_clipping_triangulation.h>
#include <micro-tess/bezier_patch_tesselator.h>
#include <micro-tess/dynamic_array.h>
#endif
#include "functions/minmax.h"
#include "functions/clamp.h"
#include "functions/swap.h"
#include "functions/orient2d.h"
#include "functions/bits.h"
#include "functions/distance.h"
#include "clippers/cohen_sutherland_clipper.h"
#include "clippers/homo_triangle_clipper.h"
#include "text/bitmap_font.h"

using namespace microtess::triangles;
using namespace microtess::polygons;
using namespace microgl::shading;
using namespace microgl::coder;
using namespace microgl;
//using color_t = microgl::color_t;
//template <typename number>
//using intensity = microgl::intensity<number>;
//namespace blendmode = microgl::blendmode;
//namespace porterduff = microgl::porterduff;

/**
 * use big integers for 2d rasterizer, this implies a 64 bits place holders
 * for all or most calculations inside the rasterizer. bigger integers imply
 * overflow is harder to come by
 */
#define CANVAS_OPT_USE_BIG_INT microgl::ints::uint8_t(0b00000001)
/**
 * inside the 2d rasterizer, use division for uv-mapping, this reduces
 * the number of bits used BUT is slower. Generally this HAS to be used
 * on a forced 32 bit rasterizer, in case you want a pure 32 bit integers
 * only during rasterization. Do not use it when in BIG INT mode.
 */
#define CANVAS_OPT_USE_DIVISION microgl::ints::uint8_t(0b00000010)
/**
 * inside the rasterizers, allow some bit compression in-order to minimize
 * overflow in 32 bit mode. disable it if 3d rendering for example starts
 * to jitter.
 */
#define CANVAS_OPT_COMPRESS_BITS microgl::ints::uint8_t(0b00001000)
/**
 * the 2d and 3d rasterizer can detect overflow of uv mapping, the detection
 * feature is great for debugging the rasterizer. this flag enables detection
 * and if so, exits the rendering. This is helpful for when using a 32 bit mode,
 * where overflows are likely to happen
 */
#define CANVAS_OPT_AVOID_RENDER_WITH_OVERFLOWS microgl::ints::uint8_t(0b00000100)
/**
 * use a true 32 bit mode in the 2d and 3d rasterizer, this means regular 32 bit integers
 * and also the usage of division in order to reduce overflow and also detecting
 * and exiting on overflows as they are likely to happen in 32 bit mode, if so,
 * please adjust some of the render options bits in the canvas and make sure you
 * render small geometries at a time
 */
#define CANVAS_OPT_32_BIT (CANVAS_OPT_USE_DIVISION | CANVAS_OPT_COMPRESS_BITS | \
                            CANVAS_OPT_AVOID_RENDER_WITH_OVERFLOWS)
/**
 * 32 bit mode without overflow detection
 */
#define CANVAS_OPT_32_BIT_FREE (CANVAS_OPT_USE_DIVISION | CANVAS_OPT_COMPRESS_BITS)

/**
 * 64 bit mode, includes usage of big integers
 */
#define CANVAS_OPT_64_BIT (CANVAS_OPT_USE_BIG_INT | \
                            CANVAS_OPT_AVOID_RENDER_WITH_OVERFLOWS | \
                            CANVAS_OPT_COMPRESS_BITS )
/**
 * 64 bit mode without overflow detection
 */
#define CANVAS_OPT_64_BIT_FREE (CANVAS_OPT_USE_BIG_INT | CANVAS_OPT_COMPRESS_BITS )
/**
 * default preset, includes usage of big integers
 */
#define CANVAS_OPT_default CANVAS_OPT_32_BIT_FREE

/**
 * the main canvas object:
 * - holds minimal state
 * - holds a bitmap reference
 *
 * @tparam bitmap_type the bitmap type
 * @tparam options the options bitset
 */
template<typename bitmap_type, microgl::ints::uint8_t options=CANVAS_OPT_default>
class canvas {
public:
    using rect = microgl::rect_t<int>;
    using canvas_t = canvas<bitmap_type, options>;
    using bitmap_t = bitmap_type;
    using pixel = typename bitmap_type::pixel;
    using pixel_coder = typename bitmap_type::pixel_coder;
    using rgba = typename pixel_coder::rgba;
    using index = unsigned int;
    using precision = unsigned char;
    using opacity_t = unsigned char;

    static constexpr bool options_compress_bits() { return options & CANVAS_OPT_COMPRESS_BITS; }
    static constexpr bool options_big_integers() { return options & CANVAS_OPT_USE_BIG_INT; }
    static constexpr bool options_avoid_overflow() { return options & CANVAS_OPT_AVOID_RENDER_WITH_OVERFLOWS; }
    static constexpr bool options_use_division() { return options & CANVAS_OPT_USE_DIVISION; }
    static constexpr bool hasNativeAlphaChannel() { return pixel_coder::rgba::a != 0;}

    // rasterizer integers
    using rint_big = microgl::ints::int64_t;
    using rint =typename microgl::traits::conditional<
            canvas_t::options_big_integers(), rint_big, microgl::ints::int32_t >::type;
    /**
     * rendering options of rasterizer
     */
    struct render_options_t {
        microgl::ints::uint8_t _2d_raster_bits_sub_pixel= options_big_integers() ? 8 : 4;
        microgl::ints::uint8_t _2d_raster_bits_uv= options_big_integers() ? 15 : 10;
        microgl::ints::uint8_t _3d_raster_bits_sub_pixel= options_big_integers() ? 8 : 4;
        microgl::ints::uint8_t _3d_raster_bits_w= options_big_integers() ? 15 : 12;
    };

    struct window_t {
        rect canvas_rect;
        rect clip_rect;
        int index_correction=0;
    };
private:

    bitmap_type _bitmap_canvas;
    window_t _window;
    render_options_t _options;

public:
    explicit canvas(bitmap_type && $bmp) : _bitmap_canvas(microgl::traits::move($bmp)) {
        updateClipRect(0, 0, $bmp.width(), $bmp.height());
        updateCanvasWindow(0, 0);
    }

    explicit canvas(const bitmap_type & $bmp) : _bitmap_canvas($bmp) {
        updateClipRect(0, 0, $bmp.width(), $bmp.height());
        updateCanvasWindow(0, 0);
    }

    canvas(int width, int height,
           const typename bitmap_type::allocator_type &allocator=typename bitmap_type::allocator_type()) :
           _bitmap_canvas(width, height, allocator) {
        updateClipRect(0, 0, width, height);
        updateCanvasWindow(0, 0);
    }

    /**
     * update the clipping rectangle of the canvas
     *
     * @param l left distance to x=0
     * @param t top distance to y=0
     * @param r right distance to x=0
     * @param b bottom distance to y=0
     */
    void updateClipRect(int l, int t, int r, int b) {
        _window.clip_rect = {l, t, r, b};
    }

    /**
     * where to position the bitmap relative to the canvas, this feature
     * can help with block rendering, where the bitmap is smaller than the canvas
     * dimensions.
     *
     * @param left relative to x=0
     * @param top relative to y=0
     * @param right relative to x=0
     * @param bottom relative to y=0
     */
    void updateCanvasWindow(int left, int top, int right, int bottom) {
        _window.canvas_rect = {left, top, left + right, top + bottom };
        _window.index_correction= _window.canvas_rect.width()*_window.canvas_rect.top
                                  + _window.canvas_rect.left;
        if(_window.clip_rect.empty())
            _window.clip_rect= _window.canvas_rect;
    }

    void updateCanvasWindow(int left, int top) {
        updateCanvasWindow(left, top, _bitmap_canvas.width(), _bitmap_canvas.height());
    }

    /**
     * given that we know the canvas size and the clip rect, calculate
     * the sub rectangle (intersection), where drawing is visible
     *
     * @return a rectangle
     */
    rect calculateEffectiveDrawRect() {
        rect r = _window.canvas_rect.intersect(_window.clip_rect);
        r.bottom-=1;r.right-=1;
        return r;
    }

    /**
     * get the clipping rectangle
     * @return rect reference
     */
    const rect & clipRect() const {
        return _window.clip_rect;
    }

    /**
     * when using the canvas window feature, index should be offset.
     * @return the offset
     */
    inline
    int indexCorrection() const {
        return _window.index_correction;
    }

    /**
     * get the canvas rectangle, should be (0, 0, width, height), unless
     * the sub windowing feature was used.
     * @return a rectangle
     */
    const rect & canvasWindowRect() const {
        return _window.canvas_rect;
    }

    /**
     * get the rendering options
     * @return options object
     */
    render_options_t & renderingOptions() {
        return _options;
    }

    // get canvas width
    int width() const;
    // get canvas height
    int height() const;
    // get size of pixel
    unsigned int sizeofPixel() const;
    // get the pixels array from the underlying bitmap
    const pixel * pixels() const;
    pixel * pixels();
    // get a pixel by position
    pixel getPixel(int x, int y) const ;
    pixel getPixel(int index) const ;
    // decode pixel color by position
    void getPixelColor(int index, color_t & output) const;
    void getPixelColor(int x, int y, color_t & output) const;

    /**
     * get the pixel coder reference of the underlying bitmap
     * @return pixel_coder
     */
    const pixel_coder & coder() const;
    /**
     * get the underlying bitmap pointer
     */
    bitmap_type & bitmapCanvas() const;

    /**
     * clear the canvas with a color intensity
     * @tparam number the number type of the intensity
     * @param color the color intensity
     */
    template <typename number>
    void clear(const intensity<number> &color);
    /**
     * clear the canvas with a color of the same type of the canvas
     */
    void clear(const color_t &color);

    // integer blenders
    /**
     * blend and composite a given color at position to the backdrop of the canvas
     *
     * @tparam BlendMode the blend mode type
     * @tparam PorterDuff the alpha compositing type
     * @tparam a_src the bits of the alpha channel of the color
     * @param val the color to blend
     * @param index the position of where to compose in the canvas
     * @param opacity 8 bit opacity [0..255]
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            microgl::ints::uint8_t a_src>
    void blendColor(const color_t &val, int x, int y, opacity_t opacity);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            microgl::ints::uint8_t a_src>
//    __attribute__((noinline))
    static void blendColor(const color_t &val, int index, opacity_t opacity, canvas & canva) {
        // correct index position when window is not at the (0,0) costs one subtraction.
        // we use it for sampling the backdrop if needed and for writing the output pixel
        index -= canva._window.index_correction;

        // we assume that the color conforms to the same pixel-coder. but we are flexible
        // for alpha channel. if coder does not have an alpha channel, the color itself may
        // have non-zero alpha channel, for which we emulate 8-bit alpha processing and also pre
        // multiply result with alpha
        constexpr bool hasBackdropAlphaChannel = pixel_coder::rgba::a != 0;
        constexpr bool hasSrcAlphaChannel = a_src != 0;
        constexpr microgl::ints::uint8_t canvas_a_bits = hasBackdropAlphaChannel ? pixel_coder::rgba::a : (a_src ? a_src : 8);
        constexpr microgl::ints::uint8_t src_a_bits = a_src ? a_src : 8;
        constexpr microgl::ints::uint8_t alpha_bits = src_a_bits;
        constexpr unsigned int alpha_max_value = microgl::ints::uint16_t (1 << alpha_bits) - 1;
        constexpr bool is_source_over = microgl::traits::is_same<PorterDuff, porterduff::FastSourceOverOnOpaque>::value;
        constexpr bool none_compositing = microgl::traits::is_same<PorterDuff, porterduff::None<>>::value;
        constexpr bool skip_blending =microgl::traits::is_same<BlendMode, blendmode::Normal>::value;
        constexpr bool premultiply_result = !hasBackdropAlphaChannel;
        const bool skip_all= skip_blending && none_compositing && opacity == 255;
        static_assert(src_a_bits==canvas_a_bits, "src_a_bits!=canvas_a_bits");

        const color_t & src = val;
        static color_t result{};

        if(!skip_all) {
            pixel output;
            static color_t backdrop{}, blended{};
            // normal blend and none composite do not require a backdrop
            if(!(skip_blending && none_compositing))
                canva._bitmap_canvas.decode(index, backdrop); // not using getPixelColor to avoid extra subtraction

            // support compositing even if the surface is opaque.
            if(!hasBackdropAlphaChannel) backdrop.a = alpha_max_value;

            if(is_source_over && src.a==0) return;

            // if we are normal then do nothing
            if(!skip_blending && backdrop.a!=0) { //  or backdrop alpha is zero is also valid
                BlendMode::template blend<pixel_coder::rgba::r,
                        pixel_coder::rgba::g,
                        pixel_coder::rgba::b>(backdrop, src, blended);
                // if backdrop alpha!= max_alpha let's first composite the blended color, this is
                // an intermediate step before Porter-Duff
                if(backdrop.a < alpha_max_value) {
                    // if((backdrop.a ^ _max_alpha_value)) {
                    unsigned int comp = alpha_max_value - backdrop.a;
                    // this is of-course a not accurate interpolation, we should
                    // divide by 255. bit shifting is like dividing by 256 and is FASTER.
                    // you will pay a price when bit count is low, this is where the error
                    // is very noticeable.
                    blended.r = (comp * src.r + backdrop.a * blended.r) >> alpha_bits;
                    blended.g = (comp * src.g + backdrop.a * blended.g) >> alpha_bits;
                    blended.b = (comp * src.b + backdrop.a * blended.b) >> alpha_bits;
                }
            }
            else {
                // skipped blending therefore use src color
                blended.r = src.r; blended.g = src.g; blended.b = src.b;
            }

            // support alpha channel in case, source pixel does not have
            blended.a = hasSrcAlphaChannel ? src.a : alpha_max_value;

            // I fixed opacity is always 8 bits no matter what the alpha depth of the native canvas
            if(opacity < 255)
                blended.a =  (int(blended.a) * int(opacity)*int(257) + 257)>>16; // blinn method

            // finally alpha composite with Porter-Duff equations,
            // this should be zero-cost for None option with compiler optimizations
            // if we do not own a native alpha channel, then please keep the composited result
            // with premultiplied alpha, this is why we composite for None option, because it performs
            // alpha multiplication
            PorterDuff::template composite<alpha_bits, premultiply_result>(backdrop, blended, result);
            canva.coder().encode(result, output);
            canva._bitmap_canvas.writeAt(index, output);
        }
        else {
            pixel output;
            canva.coder().encode(val, output);
            canva._bitmap_canvas.writeAt(index, output);
        }
    }
    /**
     * draw an already encoded pixel at position
     */
    void drawPixel(const pixel &val, int index);
    void drawPixel(const pixel &val, int x, int y);

    // circles

    /**
     * Draw a circle shape with fill and stroke
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for fill
     * @tparam Sampler2         sampler type for stroke
     *
     * @param sampler_fill      fill sampler reference
     * @param sampler_stroke    stroke sampler reference
     * @param centerX           the center x-coord of the circle
     * @param centerY           the center y-coord of the circle
     * @param radius            the radius of the circle
     * @param stroke_size       the stroke size in pixels
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1, typename number2=number1, typename Sampler1, typename Sampler2>
    void drawCircle(const Sampler1 & sampler_fill,
                    const Sampler2 & sampler_stroke,
                    const number1 &centerX, const number1 &centerY,
                    const number1 &radius, const number1 &stroke_size, opacity_t opacity=255,
                    const number2 &u0=number2(0), const number2 &v0=number2(1),
                    const number2 &u1=number2(1), const number2 &v1=number2(0));

    /**
     * Draw a stroked arc.
     * Note: pay attention to the clock-wise argument, for example, clock-wise
     * arcs, that have negative arc will be interpreted as empty arcs. This is
     * done to support animations without quirks and bugs.
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for stroke
     *
     * @param sampler_fill      fill sampler reference
     * @param sampler_stroke    stroke sampler reference
     * @param centerX           the center x-coord of the circle
     * @param centerY           the center y-coord of the circle
     * @param radius            the radius of the circle
     * @param stroke_size       the stroke size in pixels
     * @param from_angle        arc start angle in degrees (not radians)
     * @param to_angle          arc end angle in degrees (not radians)
     * @param clock_wise        is it a clock-wise arc
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     *
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1, typename number2=number1, typename Sampler>
    void drawArc(const Sampler & sampler_fill,
                 const number1 &centerX, const number1 &centerY,
                 const number1 &radius, const number1 &stroke_size,
                 number1 from_angle, number1 to_angle,
                 bool clock_wise=true,
                 opacity_t opacity=255,
                 const number2 &u0=number2(0), const number2 &v0=number2(1),
                 const number2 &u1=number2(1), const number2 &v1=number2(0));

    /**
     * Draw a Pie.
     * Note: pay attention to the clock-wise argument, for example, clock-wise
     * arcs, that have negative arc will be interpreted as empty arcs. This is
     * done to support animations without quirks and bugs.
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for stroke
     *
     * @param sampler_fill      fill sampler reference
     * @param sampler_stroke    stroke sampler reference
     * @param centerX           the center x-coord of the circle
     * @param centerY           the center y-coord of the circle
     * @param radius            the radius of the circle
     * @param from_angle        arc start angle in degrees (not radians)
     * @param to_angle          arc end angle in degrees (not radians)
     * @param clock_wise        is it a clock-wise arc
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     *
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1, typename number2=number1, typename Sampler>
    void drawPie(const Sampler & sampler_fill,
                 const number1 &centerX, const number1 &centerY,
                 const number1 &radius,
                 number1 from_angle, number1 to_angle,
                 bool clock_wise=true,
                 opacity_t opacity=255,
                 const number2 &u0=number2(0), const number2 &v0=number2(1),
                 const number2 &u1=number2(1), const number2 &v1=number2(0));

private:

    /**
     * Draw a stroked arc or full Pie
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for stroke
     *
     * @param sampler_fill      fill sampler reference
     * @param sampler_stroke    stroke sampler reference
     * @param centerX           the center x-coord of the circle
     * @param centerY           the center y-coord of the circle
     * @param radius            the radius of the circle
     * @param stroke_size       if this is an arc, choose a stroke band
     * @param draw_pie          true for pie, false for arc
     * @param cone_ax           a.x point of cone
     * @param cone_ay           a.y point of cone
     * @param cone_ax           b.x point of cone
     * @param cone_ay           b.y point of cone
     * @param from_angle        arc start angle in degrees (not radians)
     * @param to_angle          arc end angle in degrees (not radians)
     * @param clock_wise        is it a clock-wise arc
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     * @param sub_pixel_precision
     * @param uv_p
     * @param opacity
     */
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename Sampler>
    void drawArcOrPie_internal(const Sampler &sampler_fill,
                               int centerX, int centerY,
                               int radius, int stroke_size,
                               bool full_circle, bool draw_pie,
                               int cone_ax, int cone_ay,
                               int cone_bx, int cone_by,
                               int u0, int v0,
                               int u1, int v1,
                               precision sub_pixel_precision, precision uv_p,
                               canvas::opacity_t opacity);

public:

    /**
     * Draw a rounded rectangle
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for fill
     * @tparam Sampler2         sampler type for stroke
     *
     * @param sampler_fill      fill sampler reference
     * @param sampler_stroke    stroke sampler reference
     * @param left              left distance
     * @param top               top distance
     * @param right             right distance
     * @param bottom            bottom distance
     * @param radius            radius of corners
     * @param stroke_size       stroke size in pixels
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false, typename number1,
            typename number2=number1, typename Sampler1, typename Sampler2>
    void drawRoundedRect(const Sampler1 & sampler_fill,
                         const Sampler2 & sampler_stroke,
                         const number1 &left, const number1 &top,
                         const number1 &right, const number1 &bottom,
                         const number1 &radius, const number1 &stroke_size,
                         opacity_t opacity= 255,
                         const number2 &u0= number2(0), const number2 &v0= number2(1),
                         const number2 &u1= number2(1), const number2 &v1= number2(0));

private:
    /**
     * internal method, that draws rounded rectangle after conversion to fixed pooints
     * @param sub_pixel_precision   number of bits for position precision
     * @param uv_precision          number of bits for uv precision
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename Sampler1, typename Sampler2>
    void drawRoundedRect_internal(const Sampler1 & sampler_fill,
                                  const Sampler2 & sampler_stroke,
                                  int left, int top,
                                  int right, int bottom,
                                  int radius, int stroke_size,
                                  int u0, int v0, int u1, int v1,
                                  precision sub_pixel_precision, precision uv_precision,
                                  opacity_t opacity=255);

    /**
     * internal method, that draws rectangle after conversion to fixed pooints
     * @param sub_pixel_precision   number of bits for position precision
     * @param uv_precision          number of bits for uv precision
     */
    template <typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename Sampler>
    void drawRect_internal(const Sampler &sampler,
                           int left, int top,
                           int right, int bottom,
                           int u0, int v0,
                           int u1, int v1,
                           precision sub_pixel_precision, precision uv_precision,
                           opacity_t opacity);

public:

    /**
     * Draw axis aligned rectangle
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for fill
     * @tparam Sampler2         sampler type for stroke
     *
     * @param sampler           fill sampler reference
     * @param left              left distance
     * @param top               top distance
     * @param right             right distance
     * @param bottom            bottom distance
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1, typename number2=number1, typename Sampler>
    void drawRect(const Sampler &sampler,
                  const number1 & left, const number1 & top,
                  const number1 & right, const number1 & bottom,
                  opacity_t opacity = 255,
                  const number2 & u0= number2(0), const number2 & v0= number2(1),
                  const number2 & u1= number2(1), const number2 & v1= number2(0));

    /**
     * Draw rectangle with transformation
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for fill
     * @tparam Sampler2         sampler type for stroke
     *
     * @param sampler           fill sampler reference
     * @param transform         a 3x3 matrix for transform
     * @param left              left distance
     * @param top               top distance
     * @param right             right distance
     * @param bottom            bottom distance
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1=float, typename number2=number1, typename Sampler>
    void drawRect(const Sampler &sampler,
                  const matrix_3x3<number1> &transform,
                  const number1 & left, const number1 & top,
                  const number1 & right, const number1 & bottom,
                  opacity_t opacity = 255,
                  const number2 & u0= number2(0), const number2 & v0= number2(1),
                  const number2 & u1= number2(1), const number2 & v1= number2(0));

    /**
     * Draw a quadrilateral
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for fill
     * @tparam Sampler2         sampler type for stroke
     *
     * @param sampler           fill sampler reference
     * @param v0_x              1st point x value
     * @param v0_y              1st point y value
     * @param u0                1st point u value
     * @param v0                1st point v value
     *
     * @param v1_x              2nd point x value
     * @param v1_y              2nd point y value
     * @param u1                2nd point u value
     * @param v1                2nd point v value
     *
     * @param v2_x              3rd point x value
     * @param v2_y              3rd point y value
     * @param u2                3rd point u value
     * @param v2                3rd point v value
     *
     * @param v3_x              4th point x value
     * @param v3_y              4th point y value
     * @param u3                4th point u value
     * @param v3                4th point v value
     *
     * @param opacity           opacity [0..255]
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, class Sampler>
    void drawQuadrilateral(const Sampler &sampler,
                           const number1 & v0_x, const number1 & v0_y, const number2 & u0, const number2 & v0,
                           const number1 & v1_x, const number1 & v1_y, const number2 & u1, const number2 & v1,
                           const number1 & v2_x, const number1 & v2_y, const number2 & u2, const number2 & v2,
                           const number1 & v3_x, const number1 & v3_y, const number2 & u3, const number2 & v3,
                           opacity_t opacity = 255);

public:

    // Triangle batches

    /**
     * Draw 2d triangle batches. Given:
     * - vertex buffer, indices buffer, indices type, draw triangles.
     * - uvs are optional, and if are nullptr, the method will compute them alone in-place.
     * - indices buffer are optional, and if are nullptr, the method will use vertex buffer as is.
     *
     * Supported batch strategies are:
     * - TRIANGLES - every 3 indices for a triangle
     * - FAN - the first index forms with every 2 other indices
     * - STRIP - the best for low memory when possible
     *
     * Indices array point to the vertex array and is an important concept, when you want
     * to draw a subset of vertices, or to construct triangles from a set of vertices
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        antialiasing flag
     * @tparam number1          number type of position
     * @tparam number2          number type of uv coords
     * @tparam Sampler1         sampler type for fill
     * @tparam Sampler2         sampler type for stroke
     *
     * @param sampler           fill sampler reference
     * @param transform         3x3 matrix transformation
     * @param vertices          pointer to points array
     * @param uvs               (Optional) pointer to uv array
     * @param indices           (Optional) pointer to indices array
     * @param boundary_buffer   (Optional) pointer to boundary buffer, used in quick anti-aliasing
     * @param size              size of indices buffer
     * @param type              type of indices buffer {TRIANGLES, FAN, TRIANGLES_STRIP}
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     */
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename Sampler>
    void drawTriangles(const Sampler & sampler,
                       const matrix_3x3<number1> &transform,
                       const vertex2<number1> *vertices= nullptr,
                       const vertex2<number2> *uvs=nullptr,
                       const index *indices= nullptr,
                       const boundary_info * boundary_buffer= nullptr,
                       index size=0,
                       enum indices type=indices::TRIANGLES,
                       opacity_t opacity=255,
                       const number2 &u0=number2(0), const number2 &v0=number2(1),
                       const number2 &u1=number2(1), const number2 &v1=number2(0));

    /**
     * Draw 3d triangle batches. Given:
     * - vertex buffer, indices buffer, indices type, draw triangles.
     * - indices buffer are optional, and if are nullptr, the method will use vertex buffer as is.
     *
     * Supported batch strategies are:
     * - TRIANGLES - every 3 indices for a triangle
     * - FAN - the first index forms with every 2 other indices
     * - STRIP - the best for low memory when possible
     *
     * Indices array point to the vertex array and is an important concept, when you want
     * to draw a subset of vertices, or to construct triangles from a set of vertices
     *
     * @tparam BlendMode            the blend mode struct
     * @tparam PorterDuff           the alpha compositing struct
     * @tparam antialias            enable/disable anti-aliasing, currently NOT supported
     * @tparam perspective_correct  enable/disable z-correction
     * @tparam depth_buffer_flag    enable/disable z-buffer
     * @tparam Shader               the type of the shader
     * @tparam depth_buffer_type    The type of the z-buffer
     *
     * @param shader                the shader reference
     * @param viewport_width        viewport width
     * @param viewport_height       viewport height
     * @param vertex_buffer         vertex buffer array pointer
     * @param indices               (Optional) indices buffer array
     * @param size                  size of indices
     * @param type                  type of indices buffer {TRIANGLES, FAN, TRIANGLES_STRIP}
     * @param culling               face culling enum {cw, ccw, none}
     * @param depth_buffer          (optional) depth buffer pointer
     * @param opacity               opacity [0..255]
     * @param depth_range_near      z-depth near range
     * @param depth_range_far       z-depth far range
     */
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias, bool perspective_correct, bool depth_buffer_flag=false,
            typename Shader, typename depth_buffer_type >
    void drawTriangles(Shader &shader,
                       int viewport_width, int viewport_height,
                       const vertex_attributes<Shader> *vertex_buffer,
                       const index *indices,
                       index size,
                       enum indices type,
                       const microtess::triangles::face_culling & culling=
                               microtess::triangles::face_culling::none,
                       depth_buffer_type *depth_buffer=(nullptr),
                       opacity_t opacity=255,
                       const shader_number<Shader>& depth_range_near=shader_number<Shader>(0),
                       const shader_number<Shader>& depth_range_far=shader_number<Shader>(1));

    /**
     * Draw a wireframe lines using wu-lines algorithm
     *
     * @tparam BlendMode            the blend mode struct
     * @tparam PorterDuff           the alpha compositing struct
     * @tparam antialias            enable/disable anti-aliasing, currently NOT supported
     * @tparam number               the number type of positions
     * @param color                 the color
     * @param transform             the 3x3 matrix transform
     * @param vertices              the vertices array pointer
     * @param indices               (Optional) the indices buffer pointer
     * @param size                  the size of the indices
     * @param type                  type of indices buffer {TRIANGLES, FAN, TRIANGLES_STRIP}
     * @param opacity               opacity [0..255]
     */
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number=float>
    void drawTrianglesWireframe(const color_t & color,
                                const matrix_3x3<number> &transform,
                                const vertex2<number> *vertices,
                                const index *indices,
                                index size,
                                enum indices type,
                                opacity_t opacity=255);

    // single triangles, includes shader based and one very fast fixed pipeline for color and textures
    /**
     * Draw a single wireframe triangle
     * @tparam number   the number type of position
     * @param color     the color
     * @param p0        1st vertex
     * @param p1        2nd vertex
     * @param p2        3rd vertex
     * @param opacity   opacity [0..255]
     */
    template<typename number=float>
    void drawTriangleWireframe(const color_t &color,
                               const vertex2<number> &p0,
                               const vertex2<number> &p1,
                               const vertex2<number> &p2,
                               opacity_t opacity=255);

private:
    /**
     * Internal triangle drawing
     *
     * @param sub_pixel_precision   bit counts for position precision
     * @param uv_precision          bits count for uv precision
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool perspective_correct=false,
            typename Sampler>
    void drawTriangle_internal(const Sampler &sample,
                               int v0_x, int v0_y, int u0, int v0, int q0,
                               int v1_x, int v1_y, int u1, int v1, int q1,
                               int v2_x, int v2_y, int u2, int v2, int q2,
                               opacity_t opacity, precision sub_pixel_precision,
                               precision uv_precision, bool aa_first_edge = true,
                               bool aa_second_edge = true, bool aa_third_edge = true);

public:
    /**
     * Draw a triangle with sampler
     *
     * @tparam BlendMode            the blend mode struct
     * @tparam PorterDuff           the alpha compositing struct
     * @tparam antialias            enable/disable anti-aliasing, currently NOT supported
     * @tparam number1              vertices number type
     * @tparam number2              uv coords number type
     * @tparam Sampler              sampler type
     *
     * @param sample                sampler reference
     * @param v0_x                  1st point x
     * @param v0_y                  1st point y
     * @param u0                    1st point u
     * @param v0                    1st point v
     *
     * @param v1_x                  2nd point x
     * @param v1_y                  2nd point y
     * @param u1                    2nd point u
     * @param v1                    2nd point v
     *
     * @param v2_x                  3rd point x
     * @param v2_y                  3rd point y
     * @param u2                    3rd point u
     * @param v2                    3rd point v
     *
     * @param opacity               opacity [0..255]
     * @param aa_first_edge         antialias 1st edge
     * @param aa_second_edge        antialias 2nd edge
     * @param aa_third_edge         antialias 3rd edge
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, typename Sampler>
    void drawTriangle(const Sampler &sample,
                      const number1 &v0_x, const number1 &v0_y, const number2 &u0, const number2 &v0,
                      const number1 &v1_x, const number1 &v1_y, const number2 &u1, const number2 &v1,
                      const number1 &v2_x, const number1 &v2_y, const number2 &u2, const number2 &v2,
                      opacity_t opacity = 255, bool aa_first_edge = true, bool aa_second_edge = true,
                      bool aa_third_edge = true);

    /**
     * Draw a triangle with 3d shader
     *
     * @tparam BlendMode            the blend mode struct
     * @tparam PorterDuff           the alpha compositing struct
     * @tparam antialias            enable/disable anti-aliasing, currently NOT supported
     * @tparam perspective_correct  enable/disable z-correction
     * @tparam depth_buffer_flag    enable/disable z-buffer
     * @tparam Shader               the type of the shader
     * @tparam depth_buffer_type    The type of the z-buffer
     *
     * @param shader                the shader reference
     * @param viewport_width        viewport width
     * @param viewport_height       viewport height
     * @param v0                    1st vertex shader attributes
     * @param v1                    2nd vertex shader attributes
     * @param v2                    3rd vertex shader attributes
     * @param opacity               opacity [0..255]
     * @param culling               face culling enum {cw, ccw, none}
     * @param depth_buffer          (optional) depth buffer pointer
     * @param depth_range_near      z-depth near range
     * @param depth_range_far       z-depth far range
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None<>,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename Shader, typename depth_buffer_type >
    void drawTriangle(Shader &shader,
                      int viewport_width, int viewport_height,
                      vertex_attributes<Shader> v0,
                      vertex_attributes<Shader> v1,
                      vertex_attributes<Shader> v2,
                      opacity_t opacity, const microtess::triangles::face_culling & culling=
                              microtess::triangles::face_culling::none,
                      depth_buffer_type * depth_buffer=nullptr,
                      const shader_number<Shader>& depth_range_near=shader_number<Shader>(0),
                      const shader_number<Shader>& depth_range_far=shader_number<Shader>(1));

private:
    /**
     * Internal draw triangle with shader
     * @tparam BlendMode            the blend mode struct
     * @tparam PorterDuff           the alpha compositing struct
     * @tparam antialias            enable/disable anti-aliasing, currently NOT supported
     * @tparam perspective_correct  enable/disable z-correction
     * @tparam depth_buffer_flag    enable/disable z-buffer
     * @tparam Shader               the type of the shader
     * @tparam number               the number type of the varying attributes
     * @tparam depth_buffer_type    The type of the z-buffer
     *
     * @param $shader               shader reference
     * @param viewport_width        viewport width
     * @param viewport_height       viewport height
     * @param p0                    1st triangle position in clip space
     * @param p1                    2nd triangle position in clip space
     * @param p2                    3rd triangle position in clip space
     * @param varying_v0            1st varying attributes
     * @param varying_v1            2nd varying attributes
     * @param varying_v2            3rd varying attributes
     * @param opacity               opacity [0..255]
     * @param culling               face culling enum {cw, ccw, none}
     * @param depth_buffer          (optional) depth buffer pointer
     * @param depth_range_near      z-depth near range
     * @param depth_range_far       z-depth far range
     */
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None<>,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename Shader, typename number, typename depth_buffer_type >
    void drawTriangle_shader_homo_internal(Shader &$shader,
                                           int viewport_width, int viewport_height,
                                           const vertex4<number> &p0,
                                           const vertex4<number> &p1,
                                           const vertex4<number> &p2,
                                           varying<Shader> varying_v0,
                                           varying<Shader> varying_v1,
                                           varying<Shader> varying_v2,
                                           opacity_t opacity, const microtess::triangles::face_culling & culling=
                                                   microtess::triangles::face_culling::none,
                                           depth_buffer_type * depth_buffer=nullptr,
                                           number depth_range_near=number(0), number depth_range_far=number(1));

public:
    // Masks
    /**
     * Draw a mask on the canvas
     *
     * @tparam number1  number type of positions
     * @tparam number2  number type of uv coords
     * @tparam Sampler  sampler type
     *
     * @param mode      the mode/channel to use from the sampler as mask
     * @param sampler   the sampler reference
     * @param left      the left position
     * @param top       the top position
     * @param right     the right position
     * @param bottom    the bottom position
     * @param u0        uv coord
     * @param v0        uv coord
     * @param u1        uv coord
     * @param v1        uv coord
     * @param opacity   opacity [0..255]
     */
    template <typename number1, typename number2=number1, typename Sampler>
    void drawMask(const masks::chrome_mode &mode,
                  const Sampler &sampler,
                  number1 left, number1 top,
                  number1 right, number1 bottom,
                  number2 u0=number2(0), number2 v0=number2(1),
                  number2 u1=number2(1), number2 v1=number2(0),
                  opacity_t opacity = 255);

private:
    /**
     * Internal Draw a mask on the canvas
     *
     * @param sub_pixel_precision   bit count for position precision
     * @param uv_precision          bit count for uv coords precision
     */
    template <typename Sampler>
    void drawMask_internal(const masks::chrome_mode &mode,
                           const Sampler &sampler,
                           int left, int top,
                           int right, int bottom,
                           int u0, int v0,
                           int u1, int v1,
                           precision sub_pixel_precision, precision uv_precision,
                           opacity_t opacity = 255);

public:
    /**
     * Draw a Quadratic or Cubic bezier patch
     *
     * @tparam patch_type  { microtess::patch_type::BI_QUADRATIC, microtess::patch_type::BI_CUBIC } enum
     * @tparam BlendMode    the blend mode struct
     * @tparam PorterDuff   the alpha compositing struct
     * @tparam antialias    enable/disable anti-aliasing, currently NOT supported
     * @tparam debug        enable debug mode ?
     * @tparam number1      number type for vertices
     * @tparam number2      number type for uv coords
     * @tparam Sampler      type of sampler
     *
     * @param sampler       sampler reference
     * @param transform     3x3 matrix transform
     * @param mesh          4*4*2=32 or 3*3*2=18 patch, flattened array of row-major (x, y) points
     * @param uOrder        3 or 4 (quadratic vs cubic)
     * @param vOrder        3 or 4 (quadratic vs cubic)
     * @param uSamples      the number of samples to take along U axis
     * @param vSamples      the number of samples to take along V axis
     * @param u0            uv coord
     * @param v0            uv coord
     * @param u1            uv coord
     * @param v1            uv coord
     * @param opacity       opacity [0..255]
     */
    template<microtess::patch_type patch_type,
            typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool debug=false, typename number1, typename number2=number1,
            typename Sampler, class Allocator=microgl::traits::std_rebind_allocator<>>
    void drawBezierPatch(const Sampler &sampler,
                         const matrix_3x3<number1> &transform,
                         const number1 *mesh,
                         unsigned uSamples=20, unsigned vSamples=20,
                         number2 u0=number2(0), number2 v0=number2(1),
                         number2 u1=number2(1), number2 v1=number2(0),
                         opacity_t opacity=255,
                         const Allocator & allocator=Allocator());

    // polygons
    /**
     * Draw a polygon of any type via tesselation given a hint. Notes:
     * 1. Convex polygon does not require a memory allocation for the tesselation, so use convex polygons for memory efficiency.
     * 2. All other polygons types allocate memory for triangulation
     *
     * @tparam hint         the type of polygon {SIMPLE, CONCAVE, X_MONOTONE, Y_MONOTONE, CONVEX, COMPLEX, SELF_INTERSECTING}
     * @tparam BlendMode    the blend mode struct
     * @tparam PorterDuff   the alpha compositing struct
     * @tparam antialias    enable/disable anti-aliasing, currently NOT supported
     * @tparam debug        enable debug mode ?
     * @tparam number1      number type for vertices
     * @tparam number2      number type for uv coords
     * @tparam Sampler      type of sampler
     *
     * @param sampler       sampler reference
     * @param transform     3x3 matrix transform
     * @param points        vertex array pointer
     * @param size          size of vertex array
     * @param opacity       opacity [0..255]
     * @param u0            uv coord
     * @param v0            uv coord
     * @param u1            uv coord
     * @param v1            uv coord
     */
    template <microtess::polygons::hints hint=microtess::polygons::hints::SIMPLE,
            typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool debug=false,
            typename number1=float, typename number2=number1, typename Sampler,
            class tessellation_allocator=microgl::traits::std_rebind_allocator<>>
    void drawPolygon(const Sampler &sampler,
                     const matrix_3x3<number1> &transform,
                     const vertex2<number1> * points,
                     index size, opacity_t opacity=255,
                     number2 u0=number2(0), number2 v0=number2(1),
                     number2 u1=number2(1), number2 v1=number2(0),
                     const tessellation_allocator & allocator=tessellation_allocator());
private:
    /**
     * Internal WU-lines drawing
     * @param color     color
     * @param x0        1st point x
     * @param y0        1st point y
     * @param x1        2nd point x
     * @param y1        2nd point y
     * @param bits      precision for position
     * @param opacity   opacity [0..255]
     */
    void drawWuLine_internal(const color_t & color,
                             int x0, int y0, int x1, int y1,
                             precision bits = 0, opacity_t opacity= 255);

public:
    // Wu lines
    /**
     * Draw a Wu-line between two points
     *
     * @tparam number   number type of positions
     *
     * @param color     the color
     * @param x0        1st point x
     * @param y0        1st point y
     * @param x1        2nd point x
     * @param y1        2nd point y
     * @param opacity   opacity [0..255]
     */
    template<typename number>
    void drawWuLine(const color_t & color,
                    const number &x0, const number &y0, const number &x1, const number &y1,
                    opacity_t opacity=255);

    /**
     * Draw a Wu-line path
     * @tparam number       number type for positions
     *
     * @param color         the color
     * @param points        the points array pointer
     * @param size          the size of the points array
     * @param closed_path   is the path closed ?
     */
    template <typename number>
    void drawWuLinePath(const color_t & color,
                        const vertex2<number> *points,
                        unsigned int size = 4,
                        bool closed_path = false);

    /**
     * Draw vector Path stroke
     *
     * @tparam BlendMode            the blend mode struct
     * @tparam PorterDuff           the alpha compositing struct
     * @tparam antialias            enable/disable anti-aliasing, currently NOT supported
     * @tparam debug                debug mode ?
     * @tparam number1              number type of path
     * @tparam number2              number type of uv coords
     * @tparam Sampler              Sampler type
     * @tparam Iterable Any numbers iterable container (implements begin()/)end())
     * @tparam path_container_template the template of the container used by path
     *
     * @param sampler               sampler reference
     * @param transform             3x3 matrix for transform
     * @param path                  the path reference
     * @param stroke_width          stroke width in pixels
     * @param cap                   stroke cap enum {butt, round, square}
     * @param line_join             stroke line join {none, miter, miter_clip, round, bevel}
     * @param miter_limit           the miter limit
     * @param stroke_dash_array     stroke dash pattern
     * @param stroke_dash_offset    stroke dash offset
     * @param opacity               opacity [0..255]
     * @param u0                    uv coord
     * @param v0                    uv coord
     * @param u1                    uv coord
     * @param v1                    uv coord
     */
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool debug=false,
            typename number1=float, typename number2=float,
            typename Sampler, class Iterable, template<typename...> class path_container_template,
            class tessellation_allocator=microgl::traits::std_rebind_allocator<>>
    void drawPathStroke(const Sampler &sampler,
                        const matrix_3x3<number1> &transform,
                        microtess::path<number1, path_container_template, tessellation_allocator> &path,
                        const number1 &stroke_width=number1(1),
                        const microtess::stroke_cap &cap=microtess::stroke_cap::butt,
                        const microtess::stroke_line_join &line_join=microtess::stroke_line_join::bevel,
                        int miter_limit=4,
                        const Iterable & stroke_dash_array={},
                        int stroke_dash_offset=0, opacity_t opacity=255,
                        number2 u0=number2(0), number2 v0=number2(1),
                        number2 u1=number2(1), number2 v1=number2(0));

    /**
     * Draw a vector graphics path fill
     *
     * @tparam BlendMode        the blend mode struct
     * @tparam PorterDuff       the alpha compositing struct
     * @tparam antialias        enable/disable anti-aliasing, currently NOT supported
     * @tparam debug            debug mode ?
     * @tparam number1          number type of path
     * @tparam number2          number type of uv coords
     * @tparam Sampler          Sampler type
     * @tparam path_container_template the template of the container used by path
     * @tparam tessellation_allocator the allocator used for the tessellation computation
     *
     * @param sampler           sampler reference
     * @param transform         3x3 matrix for transform
     * @param path              the path reference
     * @param rule              fill rule {non_zero, even_odd}
     * @param quality           quality of tessellation {fine, better, prettier_with_extra_vertices}
     * @param opacity           opacity [0..255]
     * @param u0                uv coord
     * @param v0                uv coord
     * @param u1                uv coord
     * @param v1                uv coord
     */
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool debug=false,
            typename number1=float, typename number2=float,
            typename Sampler, template<typename...> class path_container_template,
            class tessellation_allocator=microgl::traits::std_rebind_allocator<>>
    void drawPathFill(const Sampler &sampler,
                      const matrix_3x3<number1> &transform,
                      microtess::path<number1, path_container_template, tessellation_allocator> &path,
                      const microtess::fill_rule &rule=microtess::fill_rule::non_zero,
                      const microtess::tess_quality &quality=microtess::tess_quality::better,
                      opacity_t opacity=255,
                      number2 u0=number2(0), number2 v0=number2(1),
                      number2 u1=number2(1), number2 v1=number2(0));

    /**
     * Draw Bitmap Fonts Text
     *
     * @tparam tint enable font tinting ?
     * @tparam smooth enable font smooth interpolation if font has scaled ?
     * @tparam bitmap_font_type the type of the font bitmap
     *
     * @param text char array string of text to draw
     * @param font the bitmap font reference
     * @param color the color if tinting was enabled
     * @param format text format
     * @param left left position
     * @param top top position
     * @param right right position
     * @param bottom bottom position
     * @param frame draw a frame ?
     * @param opacity opacity [0..255]
     */
    template<bool tint=true, bool smooth=false, bool frame=false, typename bitmap_font_type>
    void drawText(const char *text, microgl::text::bitmap_font<bitmap_font_type> &font, const color_t & color,
                  microgl::text::text_format & format,
                  int left, int top, int right, int bottom, opacity_t opacity=255);

};

#include "canvas.tpp"
