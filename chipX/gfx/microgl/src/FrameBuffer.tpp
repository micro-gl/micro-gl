
#include "../include/FrameBuffer.h"

template<typename T>
FrameBuffer<T>::FrameBuffer(int w, int h, PixelFormat format) : _width{w}, _height{h}, _format{format}{
    _pixels = new T[_width*_height];
}

template<typename T>
FrameBuffer<T>::~FrameBuffer() {
    delete [] _pixels;
}

template<typename T>
int FrameBuffer<T>::width() {
    return _width;
}

template<typename T>
int FrameBuffer<T>::height() {
    return _height;
}

template<typename T>
T &FrameBuffer<T>::readAt(int x, int y) {
    return _pixels[indexOf(x, y)];
}

template<typename T>
void FrameBuffer<T>::writeAt(const T &value, int x, int y) {
    _pixels[y*_width + x] = value;
}

template<typename T>
const T &FrameBuffer<T>::operator()(int x, int y) const {
    return readAt(x, y);
}

template<typename T>
T *FrameBuffer<T>::pixels() {
    return _pixels;
}

template<typename T>
void FrameBuffer<T>::fill(const T value) {
//    memset(_pixels, value, _width*_height* sizeof(T));
    std::fill(_pixels, &_pixels[_width*_height], value);
}

template<typename T>
int FrameBuffer<T>::indexOf(int x, int y) {
    return y*_width + x;
}

template<typename T>
PixelFormat FrameBuffer<T>::format() {
    return _format;
}
