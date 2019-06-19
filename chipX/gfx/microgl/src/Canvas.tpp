#include "../include/Canvas.h"

template<typename T>
Canvas<T>::Canvas(FrameBuffer<T> *frameBuffer) : _fb(frameBuffer) {
    _flag_hasAlphaChannel = (pixelFormat()==PixelFormat::RGBA8888) ||
                            (pixelFormat()==PixelFormat::RGBA4444) ||
                            (pixelFormat()==PixelFormat::RGBA5551);
}

template<typename T>
Canvas<T>::Canvas(int width, int height, PixelFormat format) :
            Canvas<T>(new FrameBuffer<T>(width, height, format)) {

}

template<typename T>
PixelFormat Canvas<T>::pixelFormat() {
    return _fb->format();
}

template<typename T>
unsigned int Canvas<T>::sizeofPixel() {
    return sizeof(T{});
}

template<typename T>
T Canvas<T>::getPixel(int x, int y) {
    return _fb->readAt(x, y);
}

template<typename T>
bool Canvas<T>::hasAlphaChannel() {
    return _flag_hasAlphaChannel;
}

template<typename T>
bool Canvas<T>::hasAntialiasing() {
    return _flag_antiAlias;
}

template<typename T>
void Canvas<T>::setAntialiasing(bool value) {
    _flag_antiAlias = value;
}

template<typename T>
color_f_t Canvas<T>::getPixelColor(int x, int y) {
    return decodeFloatRGB(getPixel(x, y), pixelFormat());
}

template<typename T>
int Canvas<T>::width() {
    return _fb->width();
}

template<typename T>
int Canvas<T>::height() {
    return _fb->height();
}

template<typename T>
void Canvas<T>::setBlendMode(const BlendMode &mode) {
    _blend_mode = mode;
}

template<typename T>
void Canvas<T>::setPorterDuffMode(const PorterDuff &mode) {
    _porter_duff_mode = mode;
}

template<typename T>
BlendMode &Canvas<T>::getBlendMode() {
    return _blend_mode;
}

template<typename T>
PorterDuff &Canvas<T>::getPorterDuffMode() {
    return _porter_duff_mode;
}

template<typename T>
inline void Canvas<T>::drawPixel(const color_f_t & val, int x, int y) {
    color_f_t result = val;

    if(hasAlphaChannel()) {
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

    drawPixel(encodeFloatRGB(result, pixelFormat()), x, y);
}

template<typename T>
void Canvas<T>::drawPixel(const T & val, int x, int y) {


    _fb->writeAt(val, x, y);
}

template<typename T>
T *Canvas<T>::pixels() {
    return _fb->pixels();
}

template<typename T>
void Canvas<T>::clear(const color_f_t &color) {
    _fb->fill(encodeFloatRGB(color, pixelFormat()));
}

template<typename T>
void Canvas<T>::drawQuad(const color_f_t & color, int left, int top, int w, int h) {
    for (int y = top; y < top + h; ++y) {
        for (int x = left; x < left + w; ++x) {
            drawPixel(color, x, y);
        }

    }

}

template<typename T>
void Canvas<T>::drawCircle(const color_f_t & color,
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

template<typename T>
void Canvas<T>::drawGradient(const color_f_t & startColor,
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

template<typename T>
void Canvas<T>::drawTriangle(const color_f_t &color,
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

//template<typename T>
//void Canvas<T>::drawTriangle(const color_f_t &color,
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
