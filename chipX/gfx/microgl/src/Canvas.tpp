#include "../include/microgl/Canvas.h"

template<typename P>
Canvas<P>::Canvas(Bitmap<P> *$bmp)
                        : _bitmap($bmp), _width{$bmp->width()}, _height{$bmp->height()} {

    _flag_hasAlphaChannel = (pixelFormat()==PixelFormat::RGBA8888) ||
                            (pixelFormat()==PixelFormat::RGBA4444) ||
                            (pixelFormat()==PixelFormat::RGBA5551);
}

template<typename P>
Canvas<P>::Canvas(int width, int height, PixelCoder<P> * $coder) :
            Canvas<P>(new Bitmap<P>(width, height, $coder)) {

}

template<typename P>
inline PixelCoder<P> *Canvas<P>::coder() {
    return _bitmap->coder();
}

template<typename P>
inline Bitmap<P> *Canvas<P>::bitmap() {
    return _bitmap;
}

template<typename P>
PixelFormat Canvas<P>::pixelFormat() {
    return coder()->format();
}

template<typename P>
unsigned int Canvas<P>::sizeofPixel() {
    return sizeof(P{});
}

template<typename P>
P Canvas<P>::getPixel(int x, int y) {
    return _bitmap->readAt(y*_width + x);
}

template<typename P>
bool Canvas<P>::hasAlphaChannel() {
    return _flag_hasAlphaChannel;
}

template<typename P>
bool Canvas<P>::hasAntialiasing() {
    return _flag_antiAlias;
}

template<typename P>
void Canvas<P>::setAntialiasing(bool value) {
    _flag_antiAlias = value;
}

template<typename P>
color_f_t Canvas<P>::getPixelColor(int x, int y) {
    return this->_bitmap->coder()->decode_to_normalized(getPixel(x, y));
}

template<typename P>
int Canvas<P>::width() {
    return _width;
}

template<typename P>
int Canvas<P>::height() {
    return _height;
}

template<typename P>
void Canvas<P>::setBlendMode(const BlendMode &mode) {
    _blend_mode = mode;
}

template<typename P>
void Canvas<P>::setPorterDuffMode(const PorterDuff &mode) {
    _porter_duff_mode = mode;
}

template<typename P>
BlendMode &Canvas<P>::getBlendMode() {
    return _blend_mode;
}

template<typename P>
PorterDuff &Canvas<P>::getPorterDuffMode() {
    return _porter_duff_mode;
}

template<typename P>
P *Canvas<P>::pixels() {
    return _bitmap->data();
}

template<typename P>
void Canvas<P>::clear(const color_f_t &color) {
    _bitmap->fill(encodeFloatRGB(color, pixelFormat()));
}

template<typename P>
inline void Canvas<P>::drawPixel(const color_f_t & val, int x, int y) {
    color_f_t result = val;

    if(true || hasAlphaChannel()) {
        color_f_t backdrop = getPixelColor(x, y);
        const color_f_t & src = val;
        color_f_t blended = blend_mode_apply(_blend_mode, backdrop, src);

        color_f_t blend_in_place{};
        blend_in_place.r = (1.0 - backdrop.a) * src.r + backdrop.a*blended.r;
        blend_in_place.g = (1.0 - backdrop.a) * src.g + backdrop.a*blended.g;
        blend_in_place.b = (1.0 - backdrop.a) * src.b + backdrop.a*blended.b;
        blend_in_place.a = float(src.a);

        result = porter_duff_apply(_porter_duff_mode, backdrop, blend_in_place);
    }

//    int index = (y * width() + x);
    P output = coder()->encode_from_normalized(result.r, result.g, result.b, result.a);

    drawPixel(output, x, y);
//    drawPixel(encodeFloatRGB(result, pixelFormat()), x, y);
}

template<typename P>
void Canvas<P>::drawPixel(const P & val, int x, int y) {
    int index = (y * width() + x);
    _bitmap->writeAt(val, index);
}

template<typename P>
void Canvas<P>::drawPixel(const P & val, int index) {
    _bitmap->writeAt(val, index);
}


template<typename P>
void Canvas<P>::drawQuad(const color_f_t & color, int left, int top, int w, int h) {
    for (int y = top; y < top + h; ++y) {
        for (int x = left; x < left + w; ++x) {
            drawPixel(color, x, y);
        }

    }

}

template<typename P>
void Canvas<P>::drawCircle(const color_f_t & color,
                           int centerX, int centerY,
                           int radius) {
    uint8_t nSubpixelsX ,nSubpixelsY;
    color_f_t color_res = color;

    nSubpixelsX = nSubpixelsY = hasAntialiasing() ? 4 : 1;

    int x1 = centerX - radius, y1 = centerY - radius;
    int x2 = centerX + radius, y2 = centerY + radius;

    for (int y = y1; y < y2; ++y) {
        for (int x = x1; x < x2; ++x) {

            // Compute the coverage by sampling the circle at "subpixel"
            // locations and counting the number of subpixels turned on.
            float coverage = 0.0f;

            for (int subpixelY = 0; subpixelY < nSubpixelsY; subpixelY++) {
                for (int subpixelX = 0; subpixelX < nSubpixelsX; subpixelX++) {
                    // Sample the center of the subpixel.
                    float sampX = x + ((subpixelX + 0.5) / nSubpixelsX);
                    float sampY = y + ((subpixelY + 0.5) / nSubpixelsY);
                    if (insideCircle(sampX, sampY, centerX, centerY, radius))
                        coverage += 1;
                }
            }

            // Take the average of all subpixels.
            coverage /= nSubpixelsX * nSubpixelsY;

            // Quick optimization: if we're fully outside the circle,
            // we don't need to compute the fill.
            if (coverage == 0)
                continue;

            color_res.a = color.a * coverage;
            drawPixel(color_res, x, y);

        }

    }

}

template<typename P>
void Canvas<P>::drawGradient(const color_f_t & startColor,
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

        drawQuad(res, x, top, 1, h);
    }

}

template<typename P>
void Canvas<P>::drawTriangle(const color_f_t &color,
                             int v0_x, int v0_y, int v1_x,
                             int v1_y, int v2_x, int v2_y) {

    float area = edgeFunction(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);

    // bounding box
    int x1 = std::min({v0_x, v1_x, v2_x});
    int y1 = std::min({v0_y, v1_y, v2_y});
    int x2 = std::max({v0_x, v1_x, v2_x});
    int y2 = std::max({v0_y, v1_y, v2_y});

    for (uint32_t y = y1; y < y2; ++y) {
        for (uint32_t x = x1; x < x2; ++x) {
            vec3_f p = {x + 0.5f, y + 0.5f, 0};

            float w0 = edgeFunction<float>(v1_x, v1_y, v2_x, v2_y, p.x, p.y);
            float w1 = edgeFunction<float>(v2_x, v2_y, v0_x, v0_y, p.x, p.y);
            float w2 = edgeFunction<float>(v0_x, v0_y, v1_x, v1_y, p.x, p.y);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                w0 /= area;
                w1 /= area;
                w2 /= area;

//                float r = w0 * c0[0] + w1 * c1[0] + w2 * c2[0];
//                float g = w0 * c0[1] + w1 * c1[1] + w2 * c2[1];
//                float b = w0 * c0[2] + w1 * c1[2] + w2 * c2[2];

                drawPixel(color, x, y);

                /*
#ifdef PERSP_CORRECT
                float z = 1 / (w0 * v0[2] + w1 * v1[2] + w2 * v2[2]);
                // if we use perspective correct interpolation we need to
                // multiply the result of this interpolation by z, the depth
                // of the point on the 3D triangle that the pixel overlaps.
                r *= z, g *= z, b *= z;
#endif
                framebuffer[j * w + i][0] = (unsigned char)(r * 255);
                framebuffer[j * w + i][1] = (unsigned char)(g * 255);
                framebuffer[j * w + i][2] = (unsigned char)(b * 255);

                 */

            }
        }
    }

}

template<typename P>
void
Canvas<P>::drawTriangle2(Bitmap<P> & bmp,
                         int v0_x, int v0_y, float u0, float v0,
                         int v1_x, int v1_y, float u1, float v1,
                         int v2_x, int v2_y, float u2, float v2) {
    float area = edgeFunction(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);

    // bounding box
    int x1 = std::min({v0_x, v1_x, v2_x});
    int y1 = std::min({v0_y, v1_y, v2_y});
    int x2 = std::max({v0_x, v1_x, v2_x});
    int y2 = std::max({v0_y, v1_y, v2_y});

    for (uint32_t y = y1; y < y2; ++y) {
        for (uint32_t x = x1; x < x2; ++x) {
            vec3_f p = {x + 0.5f, y + 0.5f, 0};

            float w0 = edgeFunction<float>(v1_x, v1_y, v2_x, v2_y, p.x, p.y);
            float w1 = edgeFunction<float>(v2_x, v2_y, v0_x, v0_y, p.x, p.y);
            float w2 = edgeFunction<float>(v0_x, v0_y, v1_x, v1_y, p.x, p.y);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                w0 /= area;
                w1 /= area;
                w2 /= area;

                float u = w0 * u0 + w1 * u1 + w2 * u2;
                float v = w0 * v0 + w1 * v1 + w2 * v2;
                int u_i = (int)(u * (float)bmp.width());
                int v_i = (int)(v * (float)bmp.height());

                int index = (v_i * bmp.width() + u_i);


                color_f_t color{};

                // todo: also connect to compositing
                // normalize to recode it to our format later
                // get rid of this, we should only care to read
                // rgba values and should not care about the bitmap
                // internal pixels type P. this way we can use different
                // bitmap types and recode them for our purposes in case
                // their internal type is different
                P d = bmp.readAt(index);

                drawPixel(d, x, y);
//                drawPixel(color, x, y);


            }

        }

    }

}

template<typename P>
void Canvas<P>::drawQuad2(Bitmap<P> &bmp, int left, int top, int w, int h) {
    float u = 0.0, v = 0.0;

    for (int y = top; y < top + h; y++) {
        v = 1.0f-(float(y - top) / (h));
        for (int x = left; x < left + w; x++) {
            u = float(x - left) / (w);

            int v_i = v*(bmp.height()-1);
            int u_i = u*bmp.width();
            int index = v_i * bmp.width() + u_i;

            P d = bmp.readAt(index);
//            P d = bmp.pixelAt(u_i, v_i);

            drawPixel(d, x, y);
        }

    }

}



//template<typename P>
//void Canvas<P>::drawTriangle(const color_f_t &color,
//                             int v0_x, int v0_y, int v1_x,
//                             int v1_y, int v2_x, int v2_y) {
//
//    float area = edgeFunction(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
//
//    // bounding box
//    int x1 = std::min({v0_x, v1_x, v2_x});
//    int y1 = std::min({v0_y, v1_y, v2_y});
//    int x2 = std::max({v0_x, v1_x, v2_x});
//    int y2 = std::max({v0_y, v1_y, v2_y});
//
//    for (uint32_t y = y1; y < y2; ++y) {
//        for (uint32_t x = x1; x < x2; ++x) {
//            vec3_f p = {x + 0.5f, y + 0.5f, 0};
//
//            float w0 = edgeFunction<float>(v1_x, v1_y, v2_x, v2_y, p.x, p.y);
//            float w1 = edgeFunction<float>(v2_x, v2_y, v0_x, v0_y, p.x, p.y);
//            float w2 = edgeFunction<float>(v0_x, v0_y, v1_x, v1_y, p.x, p.y);
//
//            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
//                w0 /= area;
//                w1 /= area;
//                w2 /= area;
//
////                float r = w0 * c0[0] + w1 * c1[0] + w2 * c2[0];
////                float g = w0 * c0[1] + w1 * c1[1] + w2 * c2[1];
////                float b = w0 * c0[2] + w1 * c1[2] + w2 * c2[2];
//
//                drawPixel(color, x, y);
//
//                /*
//#ifdef PERSP_CORRECT
//                float z = 1 / (w0 * v0[2] + w1 * v1[2] + w2 * v2[2]);
//                // if we use perspective correct interpolation we need to
//                // multiply the result of this interpolation by z, the depth
//                // of the point on the 3D triangle that the pixel overlaps.
//                r *= z, g *= z, b *= z;
//#endif
//                framebuffer[j * w + i][0] = (unsigned char)(r * 255);
//                framebuffer[j * w + i][1] = (unsigned char)(g * 255);
//                framebuffer[j * w + i][2] = (unsigned char)(b * 255);
//
//                 */
//
//            }
//        }
//    }
//
//}
//
