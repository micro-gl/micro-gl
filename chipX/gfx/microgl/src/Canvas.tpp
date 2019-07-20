#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#include <microgl/BlendMode.h>
#include "../include/microgl/Canvas.h"

template<typename P, typename CODER>
Canvas<P, CODER>::Canvas(Bitmap<P, CODER> *$bmp)
                        : _bitmap_canvas($bmp), _width{$bmp->width()}, _height{$bmp->height()} {


    uint8_t alpha_bits = coder()->bits_per_alpha();

    _flag_hasNativeAlphaChannel = alpha_bits!=0;

    // fix alpha bits depth in case we don't natively
    // support alpha, this is correct because we want to
    // support compositing even if the surface is opaque.

    _alpha_bits_for_compositing = _flag_hasNativeAlphaChannel ? alpha_bits : 8;
    _max_alpha_value = (1<<_alpha_bits_for_compositing) - 1;
}

template<typename P, typename CODER>
Canvas<P, CODER>::Canvas(int width, int height, PixelCoder<P, CODER> * $coder) :
            Canvas<P, CODER>(new Bitmap<P, CODER>(width, height, $coder)) {

}

template<typename P, typename CODER>
inline PixelCoder<P, CODER> *Canvas<P, CODER>::coder() {
    return _bitmap_canvas->coder();
}

template<typename P, typename CODER>
inline Bitmap<P, CODER> *Canvas<P, CODER>::bitmapCanvas() {
    return _bitmap_canvas;
}

template<typename P, typename CODER>
PixelFormat Canvas<P, CODER>::pixelFormat() {
    return coder()->format();
}

template<typename P, typename CODER>
unsigned int Canvas<P, CODER>::sizeofPixel() {
    return sizeof(P{});
}

template<typename P, typename CODER>
P &Canvas<P, CODER>::getPixel(int x, int y) {
    // this is not good for high performance loop, cannot be inlined
    return _bitmap_canvas->readAt(y*_width + x);
}

template<typename P, typename CODER>
P &Canvas<P, CODER>::getPixel(int index) {
    // this is not good for high performance loop, cannot be inlined
    return _bitmap_canvas->readAt(index);
}

template<typename P, typename CODER>
bool Canvas<P, CODER>::hasNativeAlphaChannel() {
    return _flag_hasNativeAlphaChannel;
}

template<typename P, typename CODER>
bool Canvas<P, CODER>::hasAntialiasing() {
    return _flag_antiAlias;
}

template<typename P, typename CODER>
void Canvas<P, CODER>::setAntialiasing(bool value) {
    _flag_antiAlias = value;
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int x, int y, color_t & output) {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int index, color_t & output) {
    this->_bitmap_canvas->decode(index, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int x, int y, color_f_t & output) {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int index, color_f_t & output) {
    this->_bitmap_canvas->decode(index, output);
}

template<typename P, typename CODER>
int Canvas<P, CODER>::width() {
    return _width;
}

template<typename P, typename CODER>
int Canvas<P, CODER>::height() {
    return _height;
}

template<typename P, typename CODER>
P *Canvas<P, CODER>::pixels() {
    return _bitmap_canvas->data();
}

template<typename P, typename CODER>
void Canvas<P, CODER>::clear(const color_f_t &color) {
    P output;
    _bitmap_canvas->coder()->encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_f_t &val, int x, int y, float opacity) {
    blendColor<BlendMode, PorterDuff>(val, y*_width + x, opacity);
}


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_f_t &val, int index, float opacity) {
    color_f_t result;//=val;
    P output;

    if(true){
        color_f_t backdrop, blended;
        const color_f_t & src = val;

        // get backdrop color
        getPixelColor(index, backdrop);

        uint8_t alpha_bits = coder()->bits_per_alpha();

        // fix alpha bits depth in case we don't natively
        // support alpha, this is correct because we want to
        // support compositing even if the surface is opaque.
        if(alpha_bits==0) {
            backdrop.a = 1.0f;
        }

        // if blend-mode is normal or the backdrop is completely transparent
        // then we don't need to blend
        bool skip_blending = BlendMode::type()==blendmode::type::Normal || backdrop.a==0;

        // if we are normal then do nothing
        if(!skip_blending) { //  or backdrop alpha is zero is also valid

            BlendMode::blend(backdrop, src, blended);

            // if backdrop alpha!= max_alpha let's first composite the blended color, this is
            // an intermidiate step before Porter-Duff
            if(backdrop.a < 1.0f) {
                float comp = 1.0f - backdrop.a;
                blended.r = (comp * src.r + backdrop.a * blended.r);
                blended.g = (comp * src.g + backdrop.a * blended.g);
                blended.b = (comp * src.b + backdrop.a * blended.b);
            }
            else {
                // do nothing if background is opaque (backdrop alpha==max_alpha) then it will equal blended
            }

        }
        else {
            // skipped blending therefore use src color
            blended.r = src.r;
            blended.g = src.g;
            blended.b = src.b;
        }

        // preserve src alpha before Porter-Duff
        blended.a = src.a * opacity;

        // finally alpha composite with Porter-Duff equations
        PorterDuff::composite(backdrop, blended, result);

    } else
        result = val;

    coder()->encode(result, output);

    drawPixel(output, index);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_t &val, int x, int y, uint8_t opacity) {
    blendColor<BlendMode, PorterDuff>(val, y*_width + x, opacity);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_t &val, int index, uint8_t opacity) {
    color_t result;//=val;
    P output;

    if(true){
        color_t backdrop, blended;
        const color_t & src = val;

        // get backdrop color
        getPixelColor(index, backdrop);

        uint8_t alpha_bits = coder()->bits_per_alpha();

        // fix alpha bits depth in case we don't natively
        // support alpha, this is correct because we want to
        // support compositing even if the surface is opaque.
        if(!hasNativeAlphaChannel()) {
            backdrop.a = _max_alpha_value; alpha_bits = _alpha_bits_for_compositing;
        }

        // if blend-mode is normal or the backdrop is completely transparent
        // then we don't need to blend.
        // the first conditional shouldbe resolved at compile-time therefore it is zero cost !!!
        // this will help with avoiding the inner conditional of the backdrop alpha, the normal
        // blending itself is zero-cost itself, but after it there is a branching which
        // is unpredictable, therefore avoiding at compile-time is great.
        bool skip_blending = BlendMode::type()==blendmode::type::Normal || backdrop.a==0;

        // if we are normal then do nothing
        if(!skip_blending) { //  or backdrop alpha is zero is also valid

            BlendMode::blend(backdrop, src, blended,
                             coder()->bits_per_red(),
                             coder()->bits_per_green(),
                             coder()->bits_per_blue());

            // if backdrop alpha!= max_alpha let's first composite the blended color, this is
            // an intermidiate step before Porter-Duff
            if(backdrop.a < _max_alpha_value) {
                // if((backdrop.a ^ _max_alpha_value)) {
                int max_alpha = _max_alpha_value;
                unsigned int comp = max_alpha - backdrop.a;
                blended.r = (comp * src.r + backdrop.a * blended.r) >> alpha_bits;
                blended.g = (comp * src.g + backdrop.a * blended.g) >> alpha_bits;
                blended.b = (comp * src.b + backdrop.a * blended.b) >> alpha_bits;
            }
            else {
                // do nothing if background is opaque (backdrop alpha==max_alpha) then it will equal blended
            }

        }
        else {
            // skipped blending therefore use src color
            blended.r = src.r;
            blended.g = src.g;
            blended.b = src.b;
        }

        // preserve src alpha before Porter-Duff
        blended.a = src.a;

        // apply opacity
        // I fixed opacity is always 8 bits no matter what the alpha depth of the
        // native canvas
        if(opacity < 255)
            blended.a =  (blended.a * opacity) >> 8;

        // finally alpha composite with Porter-Duff equations,
        // this should be zero-cost for None option with compiler optimizations
        PorterDuff::composite(backdrop, blended, result, alpha_bits);
    } else
        result = val;

    coder()->encode(result, output);

    drawPixel(output, index);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawPixel(const P & val, int x, int y) {
    int index = (y * width() + x);
    _bitmap_canvas->writeAt(val, index);
}

template<typename P, typename CODER>
inline void Canvas<P, CODER>::drawPixel(const P & val, int index) {
    _bitmap_canvas->writeAt(val, index);
}


//template<typename P, typename CODER>
//void Canvas<P, CODER>::drawCircle(const color_f_t & color,
//                           int centerX, int centerY,
//                           int radius) {
//    uint8_t nSubpixelsX ,nSubpixelsY;
//    color_f_t color_res = color;
//
//    nSubpixelsX = nSubpixelsY = 3;//hasAntialiasing() ? 4 : 1;
//
//    int x1 = centerX - radius, y1 = centerY - radius;
//    int x2 = centerX + radius, y2 = centerY + radius;
//    int index;
//
//    for (int y = y1; y < y2; ++y) {
//        // this is an optimization instead of multiplying per pixel
//        index = y * _width;
//        for (int x = x1; x < x2; ++x) {
//
//            // Compute the coverage by sampling the circle at "subpixel"
//            // locations and counting the number of subpixels turned on.
//            float coverage = 0.0f;
//
//            for (int subpixelY = 0; subpixelY < nSubpixelsY; subpixelY++) {
//                for (int subpixelX = 0; subpixelX < nSubpixelsX; subpixelX++) {
//                    // Sample the center of the subpixel.
//                    float sampX = x + ((subpixelX + 0.5f) / nSubpixelsX);
//                    float sampY = y + ((subpixelY + 0.5f) / nSubpixelsY);
//                    if (insideCircle(sampX, sampY, centerX, centerY, radius))
//                        coverage += 1;
//                }
//            }
//
//            // Take the average of all subpixels.
//            coverage /= nSubpixelsX * nSubpixelsY;
//
//            // Quick optimization: if we're fully outside the circle,
//            // we don't need to compute the fill.
//            if (coverage == 0)
//                continue;
//
//            color_res.a = color.a * coverage;
//            blendColor(color_res, index + x);
//
//        }
//
//    }
//
//}

#include "../include/microgl/Fixed.h"





template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawCircle(const color_f_t & color,
                                  int centerX, int centerY,
                                  int radius,
                                  uint8_t opacity) {
    color_t color_int;

    coder()->convert(color, color_int);

    unsigned int bits_for_antialias_distance, max_blend_distance=0;
    unsigned int a, b, c=0;

    if(antialias) {
        bits_for_antialias_distance = 1;
        max_blend_distance = 1 << bits_for_antialias_distance;
        a = radius*radius;
        b = (radius+max_blend_distance)*(radius+max_blend_distance);
        c = b - a;
    }

    bool apply_opacity = opacity!=255;
    int delta;

    int x_min = centerX - radius - max_blend_distance, y_min = centerY - radius - max_blend_distance;
    int x_max = centerX + radius + max_blend_distance, y_max = centerY + radius + max_blend_distance;
    x_min = std::max(0, x_min); y_min = std::max(0, y_min);
    x_max = std::min(width(), x_max); y_max = std::min(height(), y_max);

    int index;

    for (int y = y_min; y < y_max; ++y) {
        index = y * _width;
        for (int x = x_min; x < x_max; ++x) {

            // 16 bit precision fixed point
            int distance = signed_distance_circle_raised_quad(x, y, centerX, centerY, radius);

            if(distance<=0)
                blendColor<BlendMode, PorterDuff>(color_int, index + x, opacity);
            else if(antialias && (delta=c-distance)>=0){

//                 scale inner to 8 bit and then convert to integer
                uint8_t blend = ((delta)<<(8))/c;

                if(apply_opacity)
                    blend = (blend*opacity)>>8;

                blendColor<BlendMode, PorterDuff>(color_int, index + x, blend);
            }

        }

    }

}


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
void Canvas<P, CODER>::drawCircleFPU(const color_f_t & color,
                                  int centerX, int centerY,
                                  int radius,
                                  float opacity) {
    unsigned int max_blend = 2;

    int x_min = centerX - radius - max_blend, y_min = centerY - radius - max_blend;
    int x_max = centerX + radius + max_blend, y_max = centerY + radius + max_blend;
    int index;

    for (int y = y_min; y < y_max; ++y) {
        index = y * _width;
        for (int x = x_min; x < x_max; ++x) {

            // 16 bit precision fixed point
            float distance = sdCircle_float(x, y, centerX, centerY, radius);

            if(distance<=0)
                blendColor<BlendMode, PorterDuff>(color, index + x, opacity);
            else if(distance<=max_blend){
                // float point version
                float blend = opacity*smoothstep(max_blend, 0, distance);

                blendColor<BlendMode, PorterDuff>(color, index + x, blend);
            }

        }

    }

}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawGradient(const color_f_t & startColor,
                             const color_f_t & endColor,
                             int left, int top, int w, int h) {
    float t;
    color_f_t res{};

    for (int x = left; x < left + w; ++x) {

        t = float(x - left) / (w);

        float r = lerp(startColor.r, endColor.r, t);
        float g = lerp(startColor.g, endColor.g, t);
        float b = lerp(startColor.b, endColor.b, t);
        float a = lerp(startColor.a, endColor.a, t);

        res = {r, g, b, a};

        drawQuad(res, x, top, x+ 1, top+h);
    }

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawTriangle(const color_f_t &color,
                                       const fixed_signed v0_x, const fixed_signed v0_y,
                                       const fixed_signed v1_x, const fixed_signed v1_y,
                                       const fixed_signed v2_x, const fixed_signed v2_y,
                                       const uint8_t opacity,
                                       const uint8_t sub_pixel_precision) {
    color_t color_int;
    coder()->convert(color, color_int);

    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t MAX_BITS_FOR_PROCESSING_PRECISION = 16;
    uint8_t PR = MAX_BITS_FOR_PROCESSING_PRECISION - sub_pixel_precision;
    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    // bounding box
    int minX = (std::min({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (std::min({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (std::max({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (std::max({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;

    if(antialias) {
        bits_distance = 0;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PR;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
//        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
//        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y}, sub_pixel_precision);
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y}, sub_pixel_precision);

    // Triangle setup
    int A01 = int_to_fixed_2(v0_y - v1_y, PR)/length_w0, B01 = int_to_fixed_2(v1_x - v0_x, PR)/length_w0;
    int A12 = int_to_fixed_2(v1_y - v2_y, PR)/length_w1, B12 = int_to_fixed_2(v2_x - v1_x, PR)/length_w1;
    int A20 = int_to_fixed_2(v2_y - v0_y, PR)/length_w2, B20 = int_to_fixed_2(v0_x - v2_x, PR)/length_w2;

    // Barycentric coordinates at minX/minY corner
    vec2_fixed_signed p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };
    vec2_32i p = { minX, minY };

    // reshape
    int w0_row = ((long)int_to_fixed_2(orient2d(vec2_fixed_signed{v0_x, v0_y}, vec2_fixed_signed{v1_x, v1_y}, p_fixed, sub_pixel_precision), PR))/length_w0;
    int w1_row = ((long)int_to_fixed_2(orient2d(vec2_fixed_signed{v1_x, v1_y}, vec2_fixed_signed{v2_x, v2_y}, p_fixed, sub_pixel_precision), PR))/length_w1;
    int w2_row = ((long)int_to_fixed_2(orient2d(vec2_fixed_signed{v2_x, v2_y}, vec2_fixed_signed{v0_x, v0_y}, p_fixed, sub_pixel_precision), PR))/length_w2;


    //
    // distance to edge is always h= (2*A)/L, where:
    // h=distance from point to edge
    // A = triangle area spanned by point and edge area
    // L = length of the edge
    // this simple geometric identity can be derived from
    // area of triangle equation. We are going to interpolate
    // the quantity h and we would like to evaluate h.
    // NOTE:: this is a cheap way to calculate anti-alias with
    // perpendicular distance, this is of course not correct for
    // points that are "beyond" the edges. The real calculation
    // has to use distance to points hence a square root function
    // which is expensive for integer version. This version seems to
    // work best with minimal artifacts when used with bits_distance=0 or 1.
    //
    // we interpolate in scaled precision so watch out. All of the working
    // calculations are in 15 or 16 bits precision.

    // watch out for negative values
    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;


        for (p.x = minX; p.x <= maxX; p.x++) {

            if ((w0 | w1 | w2) >= 0) {
                blendColor<BlendMode, PorterDuff>(color_int, index + p.x, opacity);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + max_distance_scaled_space_anti_alias;

                if (delta >= 0) {
                    // take the complement and rescale
                    uint8_t blend = ((long)((delta) << (8-bits_distance)))>>PR;

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
                    }

                    blendColor<BlendMode, PorterDuff>(color_int, index + p.x, blend);
                }

            }

            // One step to the right
            w0 += A01;
            w1 += A12;
            w2 += A20;

        }

        // One row step
        w0_row += B01;
        w1_row += B12;
        w2_row += B20;
        index += _width;
    }

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawTriangle(const color_f_t &color,
                                       const float v0_x, const float v0_y,
                                       const float v1_x, const float v1_y,
                                       const float v2_x, const float v2_y,
                                       const uint8_t opacity) {

    uint8_t precision = 4;
    fixed_signed v0_x_ = float_to_fixed_2(v0_x, precision);
    fixed_signed v0_y_ = float_to_fixed_2(v0_y, precision);
    fixed_signed v1_x_ = float_to_fixed_2(v1_x, precision);
    fixed_signed v1_y_ = float_to_fixed_2(v1_y, precision);
    fixed_signed v2_x_ = float_to_fixed_2(v2_x, precision);
    fixed_signed v2_y_ = float_to_fixed_2(v2_y, precision);

    drawTriangle<BlendMode, PorterDuff, antialias>(color,
            v0_x_, v0_y_,
            v1_x_, v1_y_,
            v2_x_, v2_y_,
            opacity, precision);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawTriangle(const color_f_t &color,
                                    const int v0_x, const int v0_y,
                                    const int v1_x, const int v1_y,
                                    const int v2_x, const int v2_y,
                                    const uint8_t opacity) {

    drawTriangle<BlendMode, PorterDuff, antialias>(color,
            v0_x, v0_y,
            v1_x, v1_y,
            v2_x, v2_y,
            opacity, 0);
}


inline int clamp(int val, int e0, int e1) {
    return std::min(std::max(val,e0),e1);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, bool perspective_correct,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               const fixed_signed v0_x, const fixed_signed v0_y, fixed_signed u0, fixed_signed v0, fixed_signed q0,
                               const fixed_signed v1_x, const fixed_signed v1_y, fixed_signed u1, fixed_signed v1, fixed_signed q1,
                               const fixed_signed v2_x, const fixed_signed v2_y, fixed_signed u2, fixed_signed v2, fixed_signed q2,
                               const uint8_t opacity, const uint8_t sub_pixel_precision, const uint8_t uv_precision) {

    fixed_signed area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    int bmp_width = bmp.width();

    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t BITS_COORDS_INTEGRAL = 10 + 1;
    uint8_t BITS_COORDS_FRAC = sub_pixel_precision;
    uint8_t BITS_BITMAP_W = 8;
    uint8_t BITS_BITMAP_H = 8;
    uint8_t BITS_UV_COORDS = uv_precision;

    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    uint8_t BITS_DIV_PREC = 8;
    uint8_t BITS_Q_DIV_PREC = 16 - BITS_UV_COORDS;
    uint8_t PR = BITS_DIV_PREC + BITS_UV_COORDS;
    uint8_t PR_Q = BITS_Q_DIV_PREC + BITS_UV_COORDS;

    // bounding box
    int minX = (std::min({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (std::min({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (std::max({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (std::max({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;

    if(antialias) {
        bits_distance = 0;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PR;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
//        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
//        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };
    vec2_fixed_signed p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };

    int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // convert the floats to fixed point integers
    fixed_signed u0_fixed = (u0), v0_fixed = (v0);
    fixed_signed u1_fixed = (u1), v1_fixed = (v1);
    fixed_signed u2_fixed = (u2), v2_fixed = (v2);
    fixed_signed q0_fixed, q1_fixed, q2_fixed;
    uint8_t Q_bits= PR;
    if(perspective_correct) {
        q0_fixed = (q0), q1_fixed = (q1), q2_fixed = (q2);
    }

    fixed_signed A01_u2 = fixed_mul_int(u2_fixed, (long)bmp_w_max*((v0_y - v1_y)<<BITS_DIV_PREC))/area, B01_u2 = fixed_mul_int(u2_fixed, (long)bmp_w_max*((v1_x - v0_x)<<BITS_DIV_PREC))/area; // w0
    fixed_signed A12_u0 = fixed_mul_int(u0_fixed, (long)bmp_w_max*((v1_y - v2_y)<<BITS_DIV_PREC))/area, B12_u0 = fixed_mul_int(u0_fixed, (long)bmp_w_max*((v2_x - v1_x)<<BITS_DIV_PREC))/area; // w1
    fixed_signed A20_u1 = fixed_mul_int(u1_fixed, (long)bmp_w_max*((v2_y - v0_y)<<BITS_DIV_PREC))/area, B20_u1 = fixed_mul_int(u1_fixed, (long)bmp_w_max*((v0_x - v2_x)<<BITS_DIV_PREC))/area; // w2

    fixed_signed A01_v2 = fixed_mul_int(v2_fixed, (long)bmp_h_max*((v0_y - v1_y)<<BITS_DIV_PREC))/area, B01_v2 = fixed_mul_int(v2_fixed, (long)bmp_h_max*((v1_x - v0_x)<<BITS_DIV_PREC))/area; // w0
    fixed_signed A12_v0 = fixed_mul_int(v0_fixed, (long)bmp_h_max*((v1_y - v2_y)<<BITS_DIV_PREC))/area, B12_v0 = fixed_mul_int(v0_fixed, (long)bmp_h_max*((v2_x - v1_x)<<BITS_DIV_PREC))/area; // w1
    fixed_signed A20_v1 = fixed_mul_int(v1_fixed, (long)bmp_h_max*((v2_y - v0_y)<<BITS_DIV_PREC))/area, B20_v1 = fixed_mul_int(v1_fixed, (long)bmp_h_max*((v0_x - v2_x)<<BITS_DIV_PREC))/area; // w2

    fixed_signed A01_q2, A12_q0, A20_q1, B01_q2, B12_q0, B20_q1;
    if(perspective_correct) {
        A01_q2 = fixed_mul_int(q2_fixed, (long)(v0_y - v1_y)<<BITS_Q_DIV_PREC)/area, B01_q2 = fixed_mul_int(q2_fixed, (long)(v1_x - v0_x)<<BITS_Q_DIV_PREC)/area; // w0
        A12_q0 = fixed_mul_int(q0_fixed, (long)(v1_y - v2_y)<<BITS_Q_DIV_PREC)/area, B12_q0 = fixed_mul_int(q0_fixed, (long)(v2_x - v1_x)<<BITS_Q_DIV_PREC)/area; // w1
        A20_q1 = fixed_mul_int(q1_fixed, (long)(v2_y - v0_y)<<BITS_Q_DIV_PREC)/area, B20_q1 = fixed_mul_int(q1_fixed, (long)(v0_x - v2_x)<<BITS_Q_DIV_PREC)/area; // w2
    }

    // this can produce a 2P bits number if the points form a a perpendicular triangle
    fixed_signed area_v1_v2_p = orient2d({v1_x, v1_y}, {v2_x, v2_y}, p_fixed, sub_pixel_precision),
                area_v2_v0_p = orient2d({v2_x, v2_y}, {v0_x, v0_y}, p_fixed, sub_pixel_precision),
                area_v0_v1_p = orient2d({v0_x, v0_y}, {v1_x, v1_y}, p_fixed, sub_pixel_precision);

    fixed_signed w1_row_u = fixed_mul_int(u0_fixed, (long)bmp_w_max*(area_v1_v2_p<<BITS_DIV_PREC))/area;
    fixed_signed w2_row_u = fixed_mul_int(u1_fixed, (long)bmp_w_max*(area_v2_v0_p<<BITS_DIV_PREC))/area;
    fixed_signed w0_row_u = fixed_mul_int(u2_fixed, (long)bmp_w_max*(area_v0_v1_p<<BITS_DIV_PREC))/area;

    fixed_signed w1_row_v = fixed_mul_int(v0_fixed, (long)bmp_h_max*(area_v1_v2_p<<BITS_DIV_PREC))/area;
    fixed_signed w2_row_v = fixed_mul_int(v1_fixed, (long)bmp_h_max*(area_v2_v0_p<<BITS_DIV_PREC))/area;
    fixed_signed w0_row_v = fixed_mul_int(v2_fixed, (long)bmp_h_max*(area_v0_v1_p<<BITS_DIV_PREC))/area;

    fixed_signed w1_row_q, w2_row_q, w0_row_q;

    if(perspective_correct) {
        w1_row_q = fixed_mul_int(q0_fixed, ((long)area_v1_v2_p)<<BITS_Q_DIV_PREC) / area;
        w2_row_q = fixed_mul_int(q1_fixed, ((long)area_v2_v0_p)<<BITS_Q_DIV_PREC) / area;
        w0_row_q = fixed_mul_int(q2_fixed, ((long)area_v0_v1_p)<<BITS_Q_DIV_PREC) / area;
    }

    // lengths of edges, produces a P+1 bits number
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y}, sub_pixel_precision);
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y}, sub_pixel_precision);


    // PR seems very good for the following calculations
    // Triangle setup
    // this needs at least (P+1) bits, since the delta is always <= length
    int A01 = int_to_fixed_2(v0_y - v1_y, PR)/length_w0, B01 = int_to_fixed_2(v1_x - v0_x, PR)/length_w0;
    int A12 = int_to_fixed_2(v1_y - v2_y, PR)/length_w1, B12 = int_to_fixed_2(v2_x - v1_x, PR)/length_w1;
    int A20 = int_to_fixed_2(v2_y - v0_y, PR)/length_w2, B20 = int_to_fixed_2(v0_x - v2_x, PR)/length_w2;

    // 2A/L = h, therefore the division produces a P bit number
    // we should use around 31-MAX_BITS_PRES
    int w0_row = int_to_fixed_2(area_v0_v1_p, PR)/length_w0;
    int w1_row = int_to_fixed_2(area_v1_v2_p, PR)/length_w1;
    int w2_row = int_to_fixed_2(area_v2_v0_p, PR)/length_w2;
    fixed_signed half = (1<<(PR))>>1;
    long one_extended = (((long)1)<<((PR)<<1));

    /*
    // LUT experiment
    static fixed_signed dic[(2<<17)+1];
    static bool filled=false;
    if(!filled) {
        for (int ix = 1; ix < (1<<17)+1; ++ix) {
            dic[ix] = (fixed_signed) fixed_one_over_fixed_2(ix, PR);
        }
        filled=true;
        return;
    }
     */

    //

    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        fixed_signed w0_u = w0_row_u;
        fixed_signed w1_u = w1_row_u;
        fixed_signed w2_u = w2_row_u;

        fixed_signed w0_v = w0_row_v;
        fixed_signed w1_v = w1_row_v;
        fixed_signed w2_v = w2_row_v;

        fixed_signed w0_q, w1_q, w2_q;

        if(perspective_correct) {
            w0_q = w0_row_q;
            w1_q = w1_row_q;
            w2_q = w2_row_q;
        }

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {

            if ((w0 | w1 | w2) >= 0) {

                fixed_signed u_fixed = w0_u + w1_u + w2_u + half;
                fixed_signed v_fixed = w0_v + w1_v + w2_v + half;

                if(perspective_correct) {
                    // this optimizes fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR),
                    // I basically save two shifts
                    fixed_signed q_fixed = (w0_q + w1_q + w2_q)>>(-PR+PR_Q);
                    fixed_signed qqs = one_extended/q_fixed;
//                    fixed_signed qqs = (w0_q + w1_q + w2_q-1)==0?0:fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR);//one_extended/(w0_q + w1_q + w2_q);
//                    fixed_signed qqs = dic[(w0_q + w1_q + w2_q)];
                    u_fixed = fixed_div_fixed_2(u_fixed, q_fixed, PR);
                    v_fixed = fixed_div_fixed_2(v_fixed, q_fixed, PR);

//                    u_fixed = fixed_mul_fixed_2(u_fixed, qqs, PR);
//                    v_fixed = fixed_mul_fixed_2(v_fixed, qqs, PR);
                }


                // we round the numbers, which greatly improves things
                int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
                int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

//                drawPixel(0xFF, index+p.x);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + max_distance_scaled_space_anti_alias;

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary
                    // I don't round since I don't care about it here

                    fixed_signed u_fixed = w0_u + w1_u + w2_u + half;
                    fixed_signed v_fixed = w0_v + w1_v + w2_v + half;

                    if(perspective_correct) {
                        // this optimizes fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR),
                        // I basically save two shifts
                        fixed_signed q_fixed = (w0_q + w1_q + w2_q + 0)>>(-PR+PR_Q);
                        fixed_signed qqs = one_extended/q_fixed;

//                        fixed_signed qqs = one_extended/(w0_q + w1_q + w2_q);

                        u_fixed = fixed_mul_fixed_2(u_fixed, qqs, PR);
                        v_fixed = fixed_mul_fixed_2(v_fixed, qqs, PR);
                    }

                    int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
                    int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                    int index_bmp = bmp_width *v_i + u_i;

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>PR;

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
                    }

                    color_t col_bmp;
                    bmp.decode(index_bmp, col_bmp);
                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            // One step to the right
            w0_u += A01_u2;
            w1_u += A12_u0;
            w2_u += A20_u1;

            w0_v += A01_v2;
            w1_v += A12_v0;
            w2_v += A20_v1;

            if(perspective_correct) {
                w0_q += A01_q2;
                w1_q += A12_q0;
                w2_q += A20_q1;
            }

            w0 += A01;
            w1 += A12;
            w2 += A20;
        }

        // One row step
        w0_row_u += B01_u2;
        w1_row_u += B12_u0;
        w2_row_u += B20_u1;

        w0_row_v += B01_v2;
        w1_row_v += B12_v0;
        w2_row_v += B20_v1;

        if(perspective_correct) {
            w0_row_q += B01_q2;
            w1_row_q += B12_q0;
            w2_row_q += B20_q1;
        }

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        index += _width;
    }

}

                /*
float ww0_f = ((float)orient2d({v0_x, v0_y}, {v1_x, v1_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))/area;
float ww1_f = ((float)orient2d({v1_x, v1_y}, {v2_x, v2_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))/area;
float ww2_f = ((float)orient2d({v2_x, v2_y}, {v0_x, v0_y},  {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))/area;

int PP = 16;
int ww0 = ((long)(orient2d({v0_x, v0_y}, {v1_x, v1_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))<<PP)/area;
int ww1 = ((long)(orient2d({v1_x, v1_y}, {v2_x, v2_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))<<PP)/area;
int ww2 = ((long)(orient2d({v2_x, v2_y}, {v0_x, v0_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))<<PP)/area;


int u__1_f =  ((ww0_f*int(u2) + ww1_f*int(u0) + ww2_f*int(u1))*bmp_w_max);
int v__1_f =  ((ww0_f*int(v2) + ww1_f*int(v0) + ww2_f*int(v1))*bmp_h_max);

int u__1 =  ((ww0*int(u2) + ww1*int(u0) + ww2*int(u1))*bmp_w_max)>>PP;
int v__1 =  ((ww0*int(v2) + ww1*int(v0) + ww2*int(v1))*bmp_h_max)>>PP;

// we round the numbers, which greatly improves things
//                int u_i = clamp(u__1_f, 0, bmp_w_max);
//                int v_i = clamp(v__1_f, 0, bmp_h_max);
//                int u_i = clamp(u__1, 0, bmp_w_max);
//                int v_i = clamp(v__1, 0, bmp_h_max);
//                int index_bmp = (bmp_width*v_i + u_i);

//*/


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               const float v0_x, const float v0_y, float u0, float v0,
                               const float v1_x, const float v1_y, float u1, float v1,
                               const float v2_x, const float v2_y, float u2, float v2,
                               const uint8_t opacity) {

    uint8_t prec_pixel = 4;
    uint8_t prec_uv = 5;
    fixed_signed v0_x_ = float_to_fixed_2(v0_x, prec_pixel);
    fixed_signed v0_y_ = float_to_fixed_2(v0_y, prec_pixel);
    fixed_signed v1_x_ = float_to_fixed_2(v1_x, prec_pixel);
    fixed_signed v1_y_ = float_to_fixed_2(v1_y, prec_pixel);
    fixed_signed v2_x_ = float_to_fixed_2(v2_x, prec_pixel);
    fixed_signed v2_y_ = float_to_fixed_2(v2_y, prec_pixel);

    fixed_signed u0_ = float_to_fixed_2(u0, prec_uv);
    fixed_signed v0_ = float_to_fixed_2(v0, prec_uv);
    fixed_signed u1_ = float_to_fixed_2(u1, prec_uv);
    fixed_signed v1_ = float_to_fixed_2(v1, prec_uv);
    fixed_signed u2_ = float_to_fixed_2(u2, prec_uv);
    fixed_signed v2_ = float_to_fixed_2(v2, prec_uv);
    fixed_signed q_ = float_to_fixed_2(1.0f, prec_uv);

    drawTriangle<BlendMode, PorterDuff, antialias, false>(bmp,
            v0_x_, v0_y_, u0_, v0_, q_,
            v1_x_, v1_y_, u1_, v1_, q_,
            v2_x_, v2_y_, u2_, v2_, q_,
            opacity, prec_pixel, prec_uv);

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               const int v0_x, const int v0_y, float u0, float v0,
                               const int v1_x, const int v1_y, float u1, float v1,
                               const int v2_x, const int v2_y, float u2, float v2,
                               const uint8_t opacity) {

    // draw without perspective
    drawTriangle<BlendMode, PorterDuff, antialias, false>(bmp,
                                                    v0_x, v0_y, u0, v0, 0.0f,
                                                    v1_x, v1_y, u1, v1, 0.0f,
                                                    v2_x, v2_y, u2, v2, 0.0f,
                                                    opacity, 0);

}

// Quadrilaterals

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawQuadrilateral(const Bitmap<P2, CODER2> & bmp,
                                    fixed_signed v0_x, fixed_signed v0_y, fixed_signed u0, fixed_signed v0,
                                    fixed_signed v1_x, fixed_signed v1_y, fixed_signed u1, fixed_signed v1,
                                    fixed_signed v2_x, fixed_signed v2_y, fixed_signed u2, fixed_signed v2,
                                    fixed_signed v3_x, fixed_signed v3_y, fixed_signed u3, fixed_signed v3,
                                    const uint8_t opacity, const uint8_t sub_pixel_precision,
                                    const uint8_t uv_precision) {

    int q_one = 1<<uv_precision;

//    bool isParallelogram_ = isParallelogram(p0, p1, p2, p3);
    bool isParallelogram_ = isParallelogram({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, {v3_x, v3_y});

    if(isParallelogram_) {

        if(isAxisAlignedRectangle({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, {v3_x, v3_y})) {
            drawQuad<BlendMode, PorterDuff>(bmp, v0_x, v0_y, v2_x, v2_y,
                    sub_pixel_precision, uv_precision, opacity);

            return;
        }

        // Note:: this was faster than rasterizing the two triangles
        // in the same loop for some reason.
        // todo:: turn off AA for common edge, since it causes an artifact at the boundary
        // todo:: of common edges
        drawTriangle<BlendMode, PorterDuff, antialias, false>(bmp,
                                                              v0_x, v0_y, u0, v0, q_one,
                                                              v1_x, v1_y, u1, v1, q_one,
                                                              v2_x, v2_y, u2, v2, q_one,
                                                              opacity, sub_pixel_precision,
                                                              uv_precision);

        drawTriangle<BlendMode, PorterDuff, antialias, false>(bmp,
                                                              v2_x, v2_y, u2, v2, q_one,
                                                              v3_x, v3_y, u3, v3, q_one,
                                                              v0_x, v0_y, u0, v0, q_one,
                                                              opacity, sub_pixel_precision,
                                                              uv_precision);

    } else {

        uint8_t DIV_prec = (16 - sub_pixel_precision)>>1;
        fixed_signed max = (1<<sub_pixel_precision);
        fixed_signed q0 = (1<<uv_precision), q1 = (1<<uv_precision), q2 = (1<<uv_precision), q3 = (1<<uv_precision);
        fixed_signed p0x = v0_x; fixed_signed p0y = v0_y;
        fixed_signed p1x = v1_x; fixed_signed p1y = v1_y;
        fixed_signed p2x = v2_x; fixed_signed p2y = v2_y;
        fixed_signed p3x = v3_x; fixed_signed p3y = v3_y;

        fixed_signed ax = p2x - p0x;
        fixed_signed ay = p2y - p0y;
        fixed_signed bx = p3x - p1x;
        fixed_signed by = p3y - p1y;

        fixed_signed t, s;

        fixed_signed cross = fixed_mul_fixed_2(ax, by, sub_pixel_precision) -
                             fixed_mul_fixed_2(ay, bx, sub_pixel_precision);

        if (cross != 0) {
            fixed_signed cy = p0y - p1y;
            fixed_signed cx = p0x - p1x;

            fixed_signed area_1 = fixed_mul_fixed_2(ax, cy, sub_pixel_precision) -
                                    fixed_mul_fixed_2(ay, cx, sub_pixel_precision);

            s = fixed_div_fixed_2((long)area_1<<DIV_prec, cross, sub_pixel_precision);

            if (s > 0 && s < ((long)max<<DIV_prec)) {
                fixed_signed area_2 = fixed_mul_fixed_2(bx, cy, sub_pixel_precision) -
                                        fixed_mul_fixed_2(by, cx, sub_pixel_precision);

                t = fixed_div_fixed_2((long)area_2<<DIV_prec, cross, sub_pixel_precision);

                if (t > 0 && t < ((long)max<<DIV_prec)) {

                    q0 = fixed_div_fixed_2((long)max<<(DIV_prec<<1), (max<<DIV_prec) - t, sub_pixel_precision);
                    q1 = fixed_div_fixed_2((long)max<<(DIV_prec<<1), (max<<DIV_prec) - s, sub_pixel_precision);
                    q2 = fixed_div_fixed_2((long)max<<(DIV_prec<<1), t, sub_pixel_precision);
                    q3 = fixed_div_fixed_2((long)max<<(DIV_prec<<1), s, sub_pixel_precision);

                }
            }
        }

        fixed_signed u0_q0 = fixed_mul_fixed_2(u0, q0, sub_pixel_precision)>>(DIV_prec);
        fixed_signed v0_q0 = fixed_mul_fixed_2(v0, q0, sub_pixel_precision)>>(DIV_prec);
        fixed_signed u1_q1 = fixed_mul_fixed_2(u1, q1, sub_pixel_precision)>>(DIV_prec);
        fixed_signed v1_q1 = fixed_mul_fixed_2(v1, q1, sub_pixel_precision)>>(DIV_prec);
        fixed_signed u2_q2 = fixed_mul_fixed_2(u2, q2, sub_pixel_precision)>>(DIV_prec);
        fixed_signed v2_q2 = fixed_mul_fixed_2(v2, q2, sub_pixel_precision)>>(DIV_prec);
        fixed_signed u3_q3 = fixed_mul_fixed_2(u3, q3, sub_pixel_precision)>>(DIV_prec);
        fixed_signed v3_q3 = fixed_mul_fixed_2(v3, q3, sub_pixel_precision)>>(DIV_prec);

        q0 = fixed_convert_fixed(q0, sub_pixel_precision + DIV_prec, uv_precision);
        q1 = fixed_convert_fixed(q1, sub_pixel_precision + DIV_prec, uv_precision);
        q2 = fixed_convert_fixed(q2, sub_pixel_precision + DIV_prec, uv_precision);
        q3 = fixed_convert_fixed(q3, sub_pixel_precision + DIV_prec, uv_precision);

        // perspective correct version
        drawTriangle<BlendMode, PorterDuff, antialias, true>(bmp,
                                                             v0_x, v0_y, u0_q0, v0_q0, q0,
                                                             v1_x, v1_y, u1_q1, v1_q1, q1,
                                                             v2_x, v2_y, u2_q2, v2_q2, q2,
                                                             opacity, sub_pixel_precision, uv_precision);

        drawTriangle<BlendMode, PorterDuff, antialias, true>(bmp,
                                                             v2_x, v2_y, u2_q2, v2_q2, q2,
                                                             v3_x, v3_y, u3_q3, v3_q3, q3,
                                                             v0_x, v0_y, u0_q0, v0_q0, q0,
                                                             opacity, sub_pixel_precision, uv_precision);

// */

        /*
        float q0 = 1, q1 = 1, q2 = 1, q3 = 1;

        float p0x = v0_x; float p0y = v0_y;
        float p1x = v1_x; float p1y = v1_y;
        float p2x = v2_x; float p2y = v2_y;
        float p3x = v3_x; float p3y = v3_y;

        float ax = p2x - p0x;
        float ay = p2y - p0y;
        float bx = p3x - p1x;
        float by = p3y - p1y;
        float t, s;
//    float cross = ax * by - ay * bx;
        float cross = ax * by - ay * bx;

        if (cross != 0) {
            float cy = p0y - p1y;
            float cx = p0x - p1x;

            s = float(ax * cy - ay * cx) / cross;
            if (s > 0 && s < 1) {
                t = float(bx * cy - by * cx) / cross;

                if (t > 0 && t < 1) {

                    q0 = 1 / (1 - t);
                    q1 = 1 / (1 - s);
                    q2 = 1 / t;
                    q3 = 1 / s;

                }
            }
        }

        fixed_signed u0_q0 = (u0*q0);
        fixed_signed v0_q0 = (v0* q0);
        fixed_signed u1_q1 = (u1* q1);
        fixed_signed v1_q1 = (v1* q1);
        fixed_signed u2_q2 = (u2* q2);
        fixed_signed v2_q2 = (v2* q2);
        fixed_signed u3_q3 = (u3* q3);
        fixed_signed v3_q3 = (v3* q3);

        // perspective correct version
        drawTriangle<BlendMode, PorterDuff, antialias, true>(bmp,
                                                             v0_x, v0_y, u0_q0, v0_q0, float_to_fixed_2(q0, uv_precision),
                                                             v1_x, v1_y, u1_q1, v1_q1, float_to_fixed_2(q1, uv_precision),
                                                             v2_x, v2_y, u2_q2, v2_q2, float_to_fixed_2(q2, uv_precision),
                                                             opacity, sub_pixel_precision, uv_precision);

        drawTriangle<BlendMode, PorterDuff, antialias, true>(bmp,
                                                             v2_x, v2_y, u2_q2, v2_q2, float_to_fixed_2(q2, uv_precision),
                                                             v3_x, v3_y, u3_q3, v3_q3, float_to_fixed_2(q3, uv_precision),
                                                             v0_x, v0_y, u0_q0, v0_q0, float_to_fixed_2(q0, uv_precision),
                                                             opacity, sub_pixel_precision, uv_precision);

         */

    }

}


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawQuadrilateral(const Bitmap<P2, CODER2> & bmp,
                                    float v0_x, float v0_y, float u0, float v0,
                                    float v1_x, float v1_y, float u1, float v1,
                                    float v2_x, float v2_y, float u2, float v2,
                                    float v3_x, float v3_y, float u3, float v3,
                                    const uint8_t opacity) {

    uint8_t p_s = 4;
    uint8_t p_uv = 5;

    drawQuadrilateral<BlendMode, PorterDuff, antialias>(bmp,
                      float_to_fixed_2(v0_x, p_s), float_to_fixed_2(v0_y, p_s), float_to_fixed_2(u0, p_uv), float_to_fixed_2(v0, p_uv),
                      float_to_fixed_2(v1_x, p_s), float_to_fixed_2(v1_y, p_s), float_to_fixed_2(u1, p_uv), float_to_fixed_2(v1, p_uv),
                      float_to_fixed_2(v2_x, p_s), float_to_fixed_2(v2_y, p_s), float_to_fixed_2(u2, p_uv), float_to_fixed_2(v2, p_uv),
                      float_to_fixed_2(v3_x, p_s), float_to_fixed_2(v3_y, p_s), float_to_fixed_2(u3, p_uv), float_to_fixed_2(v3, p_uv),
                      opacity, p_s, p_uv);

    return;

    /*
    uint8_t p = 2;

    vec2_32i p0{(int)float_to_fixed_2(v0_x, p), (int)float_to_fixed_2(v0_y, p)};
    vec2_32i p1{(int)float_to_fixed_2(v1_x, p), (int)float_to_fixed_2(v1_y, p)};
    vec2_32i p2{(int)float_to_fixed_2(v2_x, p), (int)float_to_fixed_2(v2_y, p)};
    vec2_32i p3{(int)float_to_fixed_2(v3_x, p), (int)float_to_fixed_2(v3_y, p)};

    bool isParallelogram_ = isParallelogram(p0, p1, p2, p3);

    if(isParallelogram_) {

//        if(isAxisAlignedRectangle(p0, p1, p2, p3)) {
//            drawQuad<BlendMode, PorterDuff>(bmp, p0.x, p0.y, p2.x, p2.y, opacity);
//
//            return;
//        }

        // Note:: this was faster than rasterizing the two triangles
        // in the same loop for some reason.
        // todo:: turn off AA for common edge, since it causes an artifact at the boundary
        // todo:: of common edges
        drawTriangle<BlendMode, PorterDuff, antialias, false>(bmp,
                                                       p0.x, p0.y, u0, v0, 1.0f,
                                                       p1.x, p1.y, u1, v1, 1.0f,
                                                       p2.x, p2.y, u2, v2, 1.0f,
                                                       opacity, p);

        drawTriangle<BlendMode, PorterDuff, antialias, false>(bmp,
                                                            p2.x, p2.y, u2, v2, 1.0f,
                                                            p3.x, p3.y, u3, v3, 1.0f,
                                                            p0.x, p0.y, u0, v0, 1.0f,
                                                            opacity, p);

    } else {
        float q0 = 1, q1 = 1, q2 = 1, q3 = 1;

        int p0x = v0_x; float p0y = v0_y;
        int p1x = v1_x; float p1y = v1_y;
        int p2x = v2_x; float p2y = v2_y;
        int p3x = v3_x; float p3y = v3_y;

        int ax = p2x - p0x;
        int ay = p2y - p0y;
        int bx = p3x - p1x;
        int by = p3y - p1y;
        float t, s;
//    float cross = ax * by - ay * bx;
        int cross = ax * by - ay * bx;

        if (cross != 0) {
            int cy = p0y - p1y;
            int cx = p0x - p1x;

            s = float(ax * cy - ay * cx) / cross;
            if (s > 0 && s < 1) {
                t = float(bx * cy - by * cx) / cross;

                if (t > 0 && t < 1) {

                    q0 = 1 / (1 - t);
                    q1 = 1 / (1 - s);
                    q2 = 1 / t;
                    q3 = 1 / s;

                }
            }
        }

        // perspective correct version
        drawTriangle<BlendMode, PorterDuff, antialias, true>(bmp,
                                                             p0.x, p0.y, u0*q0, v0*q0, q0,
                                                             p1.x, p1.y, u1*q1, v1*q1, q1,
                                                             p2.x, p2.y, u2*q2, v2*q2, q2,
                                                             opacity, p);

        drawTriangle<BlendMode, PorterDuff, antialias, true>(bmp,
                                                             p2.x, p2.y, u2*q2, v2*q2, q2,
                                                             p3.x, p3.y, u3*q3, v3*q3, q3,
                                                             p0.x, p0.y, u0*q0, v0*q0, q0,
                                                             opacity, p);

    }
*/
}


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawQuadrilateral(const color_f_t &color,
                                         const int v0_x, const int v0_y,
                                         const int v1_x, const int v1_y,
                                         const int v2_x, const int v2_y,
                                         const int v3_x, const int v3_y,
                                         const uint8_t opacity) {
    color_t color_int;
    coder()->convert(color, color_int);

    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x, v3_x});
    int minY = std::min({v0_y, v1_y, v2_y, v3_y});
    int maxX = std::max({v0_x, v1_x, v2_x, v3_x});
    int maxY = std::max({v0_y, v1_y, v2_y, v3_y});

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    unsigned int max_distance_anti_alias=0;

    if(antialias) {
        bits_distance = 1;
        max_distance_anti_alias = 1 << bits_distance;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping against canvas
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w2 = length({v2_x, v2_y}, {v3_x, v3_y});
    unsigned int length_w3 = length({v3_x, v3_y}, {v0_x, v0_y});

    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w0, B01 = int_to_fixed(v1_x - v0_x)/length_w0;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w1, B12 = int_to_fixed(v2_x - v1_x)/length_w1;
    int A23 = int_to_fixed(v2_y - v3_y)/length_w2, B23 = int_to_fixed(v3_x - v2_x)/length_w2;
    int A30 = int_to_fixed(v3_y - v0_y)/length_w3, B30 = int_to_fixed(v0_x - v3_x)/length_w3;

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    int w0_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w1;
    int w2_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v3_x, v3_y}, p)))/length_w2;
    int w3_row = ((long)int_to_fixed(orient2d({v3_x, v3_y}, {v0_x, v0_y}, p)))/length_w3;

    // watch out for negative values
    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        int w3 = w3_row;


        for (p.x = minX; p.x <= maxX; p.x++) {
            if ((w0 | w1 | w2 | w3) >= 0) {
                blendColor<BlendMode, PorterDuff>(color_int, index + p.x, opacity);

            } else if(antialias) {;// if(false){
                int distance = std::min({w0, w1, w2, w3});
                int delta = (distance) + (max_distance_anti_alias<<(16));

                if (delta >= 0) {
                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>16;

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
                    }

                    blendColor<BlendMode, PorterDuff>(color_int, index + p.x, blend);
                }

            }

            // One step to the right
            w0 += A01;
            w1 += A12;
            w2 += A23;
            w3 += A30;

        }

        // One row step
        w0_row += B01;
        w1_row += B12;
        w2_row += B23;
        w3_row += B30;

        index += _width;
    }

}


// quads

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
void Canvas<P, CODER>::drawQuad(const color_f_t & color,
                                const int left, const int top,
                                const int right, const int bottom,
                                const uint8_t opacity) {
    color_t color_int;
    this->coder()->convert(color, color_int);

    int index = top * _width;
    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            blendColor<BlendMode, PorterDuff>(color_int, index + x, opacity);
        }

        index += _width;
    }

}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff,
          typename P2, typename CODER2>
void Canvas<P, CODER>::drawQuad(const Bitmap<P2, CODER2> &bmp,
                                const fixed_signed left, const fixed_signed top,
                                const fixed_signed right, const fixed_signed bottom,
                                const uint8_t sub_pixel_precision, const uint8_t uv_precision,
                                const uint8_t opacity) {
    color_t col_bmp{};
    P converted{};

    uint8_t DIV_prec = 16 + sub_pixel_precision;
    unsigned int f_half = 0;//1<<(DIV_prec>>1);
    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    unsigned int bmp_width = bmp.width();
    unsigned int bmp_height = bmp.height();
    unsigned int bmp_w_max = bmp_width - 1;
    unsigned int bmp_h_max = bmp_height - 1;

    int left_ = (left + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int top_ = (top + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int right_ = (right + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int bottom_ = (bottom + max_sub_pixel_precision_value) >> sub_pixel_precision;

    fixed du = (right-left)==0 ? 0 : fixed_div_int(int_to_fixed_2(bmp_width-1, DIV_prec), right-left);
    fixed dv = (bottom-top)==0 ? 0 : fixed_div_int(int_to_fixed_2(bmp_height-1, DIV_prec), bottom-top);
    fixed u = -du, v = -dv;

    int u_i=0, v_i=0;
    int index_bmp, index;

    index = top_ * _width;

    for (int y = top_; y <= bottom_; y++) {
        v += dv;
        // v_i with multiplication
        v_i = (bmp_h_max - fixed_to_int_2(v + f_half, 16))*(bmp_width);

        for (int x = left_; x <= right_; x++) {
            u += du;
            u_i = fixed_to_int_2(u + f_half, 16);
            index_bmp = (v_i) + u_i;

            // decode the bitmap
            bmp.decode(index_bmp, col_bmp);
            // re-encode for a different canvas
            blendColor<BlendMode, PorterDuff>(col_bmp, index + x, opacity);
//             drawPixel(0xFF, index + x);

            //
            // TODO:: optimization note,
            // if we copy from same bitmap formats without blending/compositing, than it is
            // 10% of the running with composting etc... so use it for optimization.
//             converted = bmp.pixelAt(index_bmp);
//             drawPixel(converted, index + x);


        }
        u = -du;
        index += _width;
    }

}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawLine(const color_f_t &color, int x0, int y0, int x1, int y1) {
    int X0 = x0, Y0 = y0, X1 = x1, Y1=y1;
    color_t color_input;

    coder()->convert(color, color_input);

    unsigned int IntensityBits = 8;
    unsigned int NumLevels = 1 << IntensityBits;
    unsigned int maxIntensity = NumLevels - 1;
    unsigned int IntensityShift, ErrorAdj, ErrorAcc;
    unsigned int ErrorAccTemp, Weighting, WeightingComplementMask;
    int DeltaX, DeltaY, Temp, XDir;
    color_t color_previous;
    color_t color_output;
    P output;

    // Make sure the line runs top to bottom
    if (Y0 > Y1) {
        Temp = Y0; Y0 = Y1; Y1 = Temp;
        Temp = X0; X0 = X1; X1 = Temp;
    }

    // Draw the initial pixel, which is always exactly intersected by
    // the line and so needs no weighting
    blendColor(color_input, X0, Y0, maxIntensity);

    if ((DeltaX = X1 - X0) >= 0) {
        XDir = 1;
    } else {
        XDir = -1;
        DeltaX = -DeltaX; // make DeltaX positive
    }
    DeltaY = Y1 - Y0;

    // Special-case horizontal, vertical, and diagonal lines, which
    // require no weighting because they go right through the center of
    // every pixel
    if ((Y1 - Y0) == 0) {
        // Horizontal line
        while (DeltaX-- != 0) {
            X0 += XDir;
            blendColor(color_input, X0, Y0, maxIntensity);

        }
        return;
    }
    if (DeltaX == 0) {
        // Vertical line
        do {
            Y0++;
            blendColor(color_input, X0, Y0, maxIntensity);
        } while (--DeltaY != 0);
        return;
    }
    if (DeltaX == DeltaY) {
        // Diagonal line
        do {
            X0 += XDir;
            Y0++;
            blendColor(color_input, X0, Y0, maxIntensity);
        } while (--DeltaY != 0);
        return;
    }

    // line is not horizontal, diagonal, or vertical
    ErrorAcc = 0; // initialize the line error accumulator to 0
    // # of bits by which to shift ErrorAcc to get intensity level
    IntensityShift = 32 - IntensityBits;
    // Mask used to flip all bits in an intensity weighting, producing the
    // result (1 - intensity weighting)
    WeightingComplementMask = maxIntensity;

    // Is this an X-major or Y-major line?
    if (DeltaY > DeltaX) {
        // Y-major line; calculate 16-bit fixed-point fractional part of a
        // pixel that X advances each time Y advances 1 pixel, truncating the
        // result so that we won't overrun the endpoint along the X axis
        ErrorAdj = ((unsigned long) DeltaX << 32) / (unsigned long) DeltaY;
        // Draw all pixels other than the first and last
        while (--DeltaY) {
            ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
            ErrorAcc += ErrorAdj; // calculate error for next pixel
            if (ErrorAcc <= ErrorAccTemp) {
                // The error accumulator turned over, so advance the X coord
                X0 += XDir;
            }
            Y0++; // Y-major, so always advance Y
            // The IntensityBits most significant bits of ErrorAcc give us the
            // intensity weighting for this pixel, and the complement of the
            // weighting for the paired pixel
            Weighting = ErrorAcc >> IntensityShift;

            unsigned int mix = (Weighting ^ WeightingComplementMask);
            // this equals Weighting, but I write it like that for clarity for now
            unsigned int mix_complement = maxIntensity - mix;

            blendColor(color_input, X0, Y0, mix);
            blendColor(color_input, X0 + XDir, Y0, mix_complement);
        }

        // Draw the final pixel, which is always exactly intersected by the line
        // and so needs no weighting
        blendColor(color_input, X1, Y1, maxIntensity);
        return;
    }

    // It's an X-major line; calculate 16-bit fixed-point fractional part of a
    // pixel that Y advances each time X advances 1 pixel, truncating the
    // result to avoid overrunning the endpoint along the X axis
    ErrorAdj = ((unsigned long) DeltaY << 32) / (unsigned long) DeltaX;

    // Draw all pixels other than the first and last
    while (--DeltaX) {
        ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
        ErrorAcc += ErrorAdj; // calculate error for next pixel
        if (ErrorAcc <= ErrorAccTemp) {
            // The error accumulator turned over, so advance the Y coord
            Y0++;
        }
        X0 += XDir; // X-major, so always advance X

        // The IntensityBits most significant bits of ErrorAcc give us the
        // intensity weighting for this pixel, and the complement of the
        // weighting for the paired pixel
        Weighting = (ErrorAcc >> IntensityShift);

        // Tomer notes:
        // 1. i inverted the order because i do not use palettes like Michael.
        // 2. we can halve the multiplications, but add more verbosity and unreadable code
        //
        unsigned int mix = (Weighting ^ WeightingComplementMask);
        unsigned int mix_complement = maxIntensity - mix; // this equals Weighting

        blendColor(color_input, X0, Y0, mix);
        blendColor(color_input, X0, Y0 + 1, mix_complement);
    }

    // Draw the final pixel, which is always exactly intersected by the line
    // and so needs no weighting
    blendColor(color_input, X1, Y1, maxIntensity);
}


template<typename P, typename CODER>
void Canvas<P, CODER>::drawQuadraticBezierPath(color_f_t & color, vec2_32i *points,
                                               unsigned int size, unsigned int resolution_bits) {

    unsigned int resolution = resolution_bits;
    unsigned int resolution_double = resolution<<1;
    unsigned int N_SEG = (1 << resolution); // 64 resolution
    unsigned int i;

    vec3_32ui previous;

    for (int jx = 0; jx < size-2; jx+=2) {
        auto * point_anchor = &points[jx];

        for (i = 0; i <= N_SEG; ++i) {
            unsigned int t = i;//(double)i / (double)N_SEG;
            unsigned int comp = N_SEG - t;
            unsigned int a = comp * comp;
            unsigned int b = (t * comp) << 1;
            unsigned int c = t * t;
            unsigned int x = (a * point_anchor[0].x + b * point_anchor[1].x + c * point_anchor[2].x) >> resolution_double;
            unsigned int y = (a * point_anchor[0].y + b * point_anchor[1].y + c * point_anchor[2].y) >> resolution_double;

            if (i)
                drawLine(color, previous.x, previous.y, x, y);

            blendColor(color, x, y, 1.0f);

            previous = {x, y};
        }

    }

}


template<typename P, typename CODER>
void Canvas<P, CODER>::drawCubicBezierPath(color_f_t & color, vec2_32i *points,
                                           unsigned int size, unsigned int resolution_bits) {

    unsigned int resolution = resolution_bits;
    unsigned int resolution_triple = resolution*3;
    unsigned int N_SEG = (1 << resolution); // 64 resolution
    unsigned int i;

    vec3_32ui previous;

    for (int jx = 0; jx < size-3; jx+=3) {

        auto *point_anchor = &points[jx];

        for (i=0; i <= N_SEG; ++i)
        {

            // (n-t)^2 => n*n, t*t, n*t
            // (n-t)^3 => n*n*n, t*t*n, n*n*t, t*t*t
            //10
            // todo: we can use a LUT if using more point batches
            unsigned int t = i;//(double)i / (double)N_SEG;
            unsigned int comp = N_SEG - t;
            unsigned int comp_times_comp = comp * comp;
            unsigned int t_times_t = t * t;
            unsigned int a = comp * comp_times_comp;
            unsigned int b = 3 * (t * comp_times_comp);
            unsigned int c = 3*t_times_t*comp;
            unsigned int d = t*t_times_t;

            unsigned int x = (a * point_anchor[0].x + b * point_anchor[1].x + c * point_anchor[2].x + d * point_anchor[3].x)>>resolution_triple;
            unsigned int y = (a * point_anchor[0].y + b * point_anchor[1].y + c * point_anchor[2].y + d * point_anchor[3].y)>>resolution_triple;

            if(i)
                drawLine(color, previous.x, previous.y, x, y);

            blendColor(color, x, y, 1.0f);

            previous = {x, y};
        }


    }


}

template<typename P, typename CODER>
void
Canvas<P, CODER>::drawLinePath(color_f_t &color, vec2_32i *points,
                                unsigned int size) {

    for (int jx = 0; jx < size; jx++) {

        if(jx)
            drawLine(color, points[jx-1].x, points[jx-1].y, points[jx].x, points[jx].y);

    }

}


#pragma clang diagnostic pop
#pragma clang diagnostic pop