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
                                    const int v0_x, const int v0_y,
                                    const int v1_x, const int v1_y,
                                    const int v2_x, const int v2_y,
                                    const uint8_t opacity) {
    color_t color_int;
    coder()->convert(color, color_int);

    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x});
    int minY = std::min({v0_y, v1_y, v2_y});
    int maxX = std::max({v0_x, v1_x, v2_x});
    int maxY = std::max({v0_y, v1_y, v2_y});

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

    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y});

    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w0, B01 = int_to_fixed(v1_x - v0_x)/length_w0;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w1, B12 = int_to_fixed(v2_x - v1_x)/length_w1;
    int A20 = int_to_fixed(v2_y - v0_y)/length_w2, B20 = int_to_fixed(v0_x - v2_x)/length_w2;

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    int w0_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w1;
    int w2_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v0_x, v0_y}, p)))/length_w2;


    //
    // distance to edge is always h= (2*A)/L, where:
    // h=distance from point to edge
    // A = triangle spanned by point and edge area
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

    // watch out for negative values
    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;


        for (p.x = minX; p.x <= maxX; p.x++) {
//            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
            // same as (w0 >= 0 && w1 >= 0 && w2 >= 0), but use only MSB,
            // this turns three conditionals into one !!!
            // if all are positive>=0 then we are inside the triangle
            if ((w0 | w1 | w2) >= 0) {
                blendColor<BlendMode, PorterDuff>(color_int, index + p.x, opacity);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
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
            w2 += A20;

        }

        // One row step
        w0_row += B01;
        w1_row += B12;
        w2_row += B20;
        index += _width;
    }

}

inline int clamp(int val, int e0, int e1) {
    return std::min(std::max(val,e0),e1);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               const uint8_t opacity) {
    float area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y});
    int bmp_width = bmp.width();
    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x});
    int minY = std::min({v0_y, v1_y, v2_y});
    int maxX = std::max({v0_x, v1_x, v2_x});
    int maxY = std::max({v0_y, v1_y, v2_y});

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

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    unsigned int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // todo:: optimize all of these recurring expressions
    // Triangle setup
    fixed_signed A01_u2 = float_to_fixed(u2*bmp_w_max*(v0_y - v1_y)/area), B01_u2 = float_to_fixed(u2*bmp_w_max*(v1_x - v0_x)/area); //w2
    fixed_signed A12_u0 = float_to_fixed(u0*bmp_w_max*(v1_y - v2_y)/area), B12_u0 = float_to_fixed(u0*bmp_w_max*(v2_x - v1_x)/area); // w0
    fixed_signed A20_u1 = float_to_fixed(u1*bmp_w_max*(v2_y - v0_y)/area), B20_u1 = float_to_fixed(u1*bmp_w_max*(v0_x - v2_x)/area); // w1

    fixed_signed A01_v2 = float_to_fixed(v2*bmp_h_max*(v0_y - v1_y)/area), B01_v2 = float_to_fixed(v2*bmp_h_max*(v1_x - v0_x)/area); //w2
    fixed_signed A12_v0 = float_to_fixed(v0*bmp_h_max*(v1_y - v2_y)/area), B12_v0 = float_to_fixed(v0*bmp_h_max*(v2_x - v1_x)/area); // w0
    fixed_signed A20_v1 = float_to_fixed(v1*bmp_h_max*(v2_y - v0_y)/area), B20_v1 = float_to_fixed(v1*bmp_h_max*(v0_x - v2_x)/area); // w1

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // 0->1, 1->2, 2->0
    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // for 16 bits computer this is safe for 7 bits input [0..127] - not good
    fixed_signed w1_row_u = float_to_fixed(float(u0*bmp_w_max*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_u = float_to_fixed(float(u1*bmp_w_max*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_u = float_to_fixed(float(u2*bmp_w_max*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    fixed_signed w1_row_v = float_to_fixed(float(v0*bmp_h_max*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_v = float_to_fixed(float(v1*bmp_h_max*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_v = float_to_fixed(float(v2*bmp_h_max*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y});
//0->2, 1->0, 2->1
    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w0, B01 = int_to_fixed(v1_x - v0_x)/length_w0;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w1, B12 = int_to_fixed(v2_x - v1_x)/length_w1;
    int A20 = int_to_fixed(v2_y - v0_y)/length_w2, B20 = int_to_fixed(v0_x - v2_x)/length_w2;

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    // this is distance to edges
    int w0_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w1;
    int w2_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v0_x, v0_y}, p)))/length_w2;

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

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {
            if ((w0 | w1 | w2) >= 0) {

                //                /*
//                float ww0 = ((float)orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area;
//                float ww1 = ((float)orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area;
//                float ww2 = ((float)orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area;
//
//                float qq =  ww0*q2 + ww1*q0 + ww2*q1;
//
//                float u__1 =  (ww0*u2 + ww1*u0 + ww2*u1)/qq;
//                float v__1 =  (ww0*v2 + ww1*v0 + ww2*v1)/qq;
//                fixed_signed qqs = float_to_fixed(qq);

//                int u_i = (int)(u__1 * (float)(bmp.width()-1));
//                int v_i = bmp_width * (int)(v__1*(float)(bmp.height()-1));//(int)(v * (float)bmp.height());
//

                int u_i = fixed_to_int((w0_u + w1_u + w2_u));
                int v_i = fixed_to_int((w0_v + w1_v + w2_v));
                // i should clamp if i see artifacts
//                int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp_w_max);
//                int v_i = clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + (max_distance_anti_alias<<(16));

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary

                    int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp_w_max);
                    int v_i = clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp_h_max);
                    int index_bmp = bmp_width *v_i + u_i;

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>16;

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

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        index += _width;
    }

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0, float q0,
                               int v1_x, int v1_y, float u1, float v1, float q1,
                               int v2_x, int v2_y, float u2, float v2, float q2,
                               const uint8_t opacity) {

    float area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y});
    int bmp_width = bmp.width();
    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x});
    int minY = std::min({v0_y, v1_y, v2_y});
    int maxX = std::max({v0_x, v1_x, v2_x});
    int maxY = std::max({v0_y, v1_y, v2_y});

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

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // todo:: optimize all of these recurring expressions
    unsigned int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // Triangle setup
    fixed_signed A01_u2 = float_to_fixed(u2*bmp_w_max*(v0_y - v1_y)/area), B01_u2 = float_to_fixed(u2*bmp_w_max*(v1_x - v0_x)/area); //w2
    fixed_signed A12_u0 = float_to_fixed(u0*bmp_w_max*(v1_y - v2_y)/area), B12_u0 = float_to_fixed(u0*bmp_w_max*(v2_x - v1_x)/area); // w0
    fixed_signed A20_u1 = float_to_fixed(u1*bmp_w_max*(v2_y - v0_y)/area), B20_u1 = float_to_fixed(u1*bmp_w_max*(v0_x - v2_x)/area); // w1

    fixed_signed A01_v2 = float_to_fixed(v2*bmp_h_max*(v0_y - v1_y)/area), B01_v2 = float_to_fixed(v2*bmp_h_max*(v1_x - v0_x)/area); //w2
    fixed_signed A12_v0 = float_to_fixed(v0*bmp_h_max*(v1_y - v2_y)/area), B12_v0 = float_to_fixed(v0*bmp_h_max*(v2_x - v1_x)/area); // w0
    fixed_signed A20_v1 = float_to_fixed(v1*bmp_h_max*(v2_y - v0_y)/area), B20_v1 = float_to_fixed(v1*bmp_h_max*(v0_x - v2_x)/area); // w1

    fixed_signed A01_q2 = float_to_fixed(q2*(v0_y - v1_y)/area), B01_q2 = float_to_fixed(q2*(v1_x - v0_x)/area); //w2
    fixed_signed A12_q0 = float_to_fixed(q0*(v1_y - v2_y)/area), B12_q0 = float_to_fixed(q0*(v2_x - v1_x)/area); // w0
    fixed_signed A20_q1 = float_to_fixed(q1*(v2_y - v0_y)/area), B20_q1 = float_to_fixed(q1*(v0_x - v2_x)/area); // w1

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // 0->1, 1->2, 2->0
    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // for 16 bits computer this is safe for 7 bits input [0..127] - not good
    fixed_signed w1_row_u = float_to_fixed(float(u0*bmp_w_max*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_u = float_to_fixed(float(u1*bmp_w_max*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_u = float_to_fixed(float(u2*bmp_w_max*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    fixed_signed w1_row_v = float_to_fixed(float(v0*bmp_h_max*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_v = float_to_fixed(float(v1*bmp_h_max*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_v = float_to_fixed(float(v2*bmp_h_max*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    fixed_signed w1_row_q = float_to_fixed(float(q0*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_q = float_to_fixed(float(q1*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_q = float_to_fixed(float(q2*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y});
//0->2, 1->0, 2->1
    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w0, B01 = int_to_fixed(v1_x - v0_x)/length_w0;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w1, B12 = int_to_fixed(v2_x - v1_x)/length_w1;
    int A20 = int_to_fixed(v2_y - v0_y)/length_w2, B20 = int_to_fixed(v0_x - v2_x)/length_w2;

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    // this is distance to edges
    int w0_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w1;
    int w2_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v0_x, v0_y}, p)))/length_w2;

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

        fixed_signed w0_q = w0_row_q;
        fixed_signed w1_q = w1_row_q;
        fixed_signed w2_q = w2_row_q;

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {


            if ((w0 | w1 | w2) >= 0) {
//                /*
//                float ww0 = ((float)orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area;
//                float ww1 = ((float)orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area;
//                float ww2 = ((float)orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area;
//
//                float qq =  ww0*q2 + ww1*q0 + ww2*q1;
//
//                float u__1 =  (ww0*u2 + ww1*u0 + ww2*u1)/qq;
//                float v__1 =  (ww0*v2 + ww1*v0 + ww2*v1)/qq;
//                fixed_signed qqs = float_to_fixed(qq);

//                int u_i = (int)(u__1 * (float)(bmp.width()-1));
//                int v_i = bmp_width * (int)(v__1*(float)(bmp.height()-1));//(int)(v * (float)bmp.height());
//
                fixed_signed qqs = fixed_one_over_fixed(w0_q + w1_q + w2_q);
                int uuu = fixed_mul_fixed(w0_u + w1_u + w2_u, qqs);
                int vvv = fixed_mul_fixed(w0_v + w1_v + w2_v, qqs);
                int u_i = clamp(fixed_to_int(uuu), 0 ,bmp_w_max);
                int v_i = clamp(fixed_to_int(vvv),0, bmp_h_max);

                int index_bmp = (bmp_width * v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + (max_distance_anti_alias<<(16));

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary

                    /*
                    int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp.width() - 1);//(int)(u * (float)bmp.width());
                    int v_i = bmp_width * clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp.height() - 1);//(int)(v * (float)bmp.height());
                    int index_bmp = (v_i + u_i);
                     */

                    fixed_signed qqs = fixed_one_over_fixed(w0_q + w1_q + w2_q);
                    int uuu = fixed_mul_fixed(w0_u + w1_u + w2_u, qqs);
                    int vvv = fixed_mul_fixed(w0_v + w1_v + w2_v, qqs);
                    int u_i = clamp(fixed_to_int(uuu), 0 ,bmp_w_max);
                    int v_i = clamp(fixed_to_int(vvv),0, bmp_h_max);

                    int index_bmp = (bmp_width*v_i + u_i);

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>16;

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

            w0_q += A01_q2;
            w1_q += A12_q0;
            w2_q += A20_q1;

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

        w0_row_q += B01_q2;
        w1_row_q += B12_q0;
        w2_row_q += B20_q1;

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        index += _width;
    }

}

// Quadrilaterals

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawQuadrilateral(const Bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               int v3_x, int v3_y, float u3, float v3,
                               const uint8_t opacity) {

    vec2_32i p0 {v0_x, v0_y}, p1{v1_x, v1_y}, p2{v2_x, v2_y}, p3{v3_x, v3_y};

    bool isParallelogram_ = isParallelogram(p0, p1, p2, p3);

    if(isParallelogram_) {

        if(!isAxisAlignedRectangle(p0, p1, p2, p3)) {
            drawQuad<BlendMode, PorterDuff>(bmp, p0.x, p0.y, p2.x, p2.y, opacity);

            return;
        }

        // Note:: this was faster than rasterizing the two triangles
        // in the same loop for some reason.
        // todo:: turn off AA for common edge, since it causes an artifact at the boundary
        // todo:: of common edges
        drawTriangle<BlendMode, PorterDuff, antialias>(bmp,
                                                       v0_x, v0_y, u0, v0,
                                                       v1_x, v1_y, u1, v1,
                                                       v2_x, v2_y, u2, v2,
                                                       opacity);

        drawTriangle<BlendMode, PorterDuff, antialias>(bmp,
                                                       v2_x, v2_y, u2, v2,
                                                       v3_x, v3_y, u3, v3,
                                                       v0_x, v0_y, u0, v0,
                                                       opacity);

    } else {
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

            s = (ax * cy - ay * cx) / cross;
            if (s > 0 && s < 1) {
                t = (bx * cy - by * cx) / cross;

                if (t > 0 && t < 1) {

                    q0 = 1 / (1 - t);
                    q1 = 1 / (1 - s);
                    q2 = 1 / t;
                    q3 = 1 / s;

                }
            }
        }

        // perspective correct version
        drawTriangle<BlendMode, PorterDuff, antialias>(bmp,
                                                       v0_x, v0_y, u0*q0, v0*q0, q0,
                                                       v1_x, v1_y, u1*q1, v1*q1, q1,
                                                       v2_x, v2_y, u2*q2, v2*q2, q2,
                                                       opacity);

        drawTriangle<BlendMode, PorterDuff, antialias>(bmp,
                                                       v2_x, v2_y, u2*q2, v2*q2, q2,
                                                       v3_x, v3_y, u3*q3, v3*q3, q3,
                                                       v0_x, v0_y, u0*q0, v0*q0, q0,
                                                       opacity);

    }

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

///*

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff,
          typename P2, typename CODER2>
void Canvas<P, CODER>::drawQuad(const Bitmap<P2, CODER2> &bmp,
                                int left, int top,
                                int right, int bottom,
                                const uint8_t opacity) {
    color_t col_bmp{};
    P converted{};

    int bmp_width = (bmp.width());
    int bmp_height = (bmp.height());

    fixed du = fixed_div_int(int_to_fixed(bmp_width), right-left);
    fixed dv = fixed_div_int(int_to_fixed(bmp_height), bottom-top);
    fixed u = -du, v = -dv;

    int u_i=0, v_i=0;
    int index_bmp, index;

    index = top * _width;

    for (int y = top; y < bottom; y++) {
        v += dv;
        // v_i with multiplication
        v_i = (bmp_height - 1 - fixed_to_int(v))*(bmp_width);

        for (int x = left; x < right; x++) {
            u += du;
            u_i = fixed_to_int(u);
            index_bmp = (v_i) + u_i;

            // decode the bitmap
            bmp.decode(index_bmp, col_bmp);
            // re-encode for a different canvas
            blendColor<BlendMode, PorterDuff>(col_bmp, index + x, opacity);

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

    vec3_32i previous;

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

    vec3_32i previous;

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