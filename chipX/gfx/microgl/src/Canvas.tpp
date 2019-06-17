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
void Canvas<T>::drawPixel(const color_f_t & val, int x, int y) {
    color_f_t result = val;

//    if(_blend_mode!=BlendMode::Normal) {
    color_f_t backdrop = getPixelColor(x, y);
    const color_f_t & src = val;
    color_f_t blended = blend_mode_apply(_blend_mode, backdrop, src);

    color_f_t blend_in_place{};
    blend_in_place.r = (1.0 - backdrop.a) * src.r + backdrop.a*blended.r;
    blend_in_place.g = (1.0 - backdrop.a) * src.g + backdrop.a*blended.g;
    blend_in_place.b = (1.0 - backdrop.a) * src.b + backdrop.a*blended.b;
    blend_in_place.a = float(src.a);

    result = porter_duff_apply(_porter_duff_mode, backdrop, blend_in_place);
//    }

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
    int x1 = centerX - radius, y1 = centerY - radius;
    int x2 = centerX + radius, y2 = centerY + radius;
    // direct color, optimization
    T color_val = encodeFloatRGB(color, pixelFormat());

    for (int y = y1; y < y2; ++y) {
        for (int x = x1; x < x2; ++x) {
            float distX = (x - centerX + 0.5f), distY = (y - centerY + 0.5f);
            float distance = sqrt(distX * distX + distY * distY);
            if (distance <= radius) {
//                drawPixel(color_val, x, y);
                drawPixel(color, x, y);
            }

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

