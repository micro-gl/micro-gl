
#include "../include/microgl/FrameBuffer.h"

template<typename T>
FrameBuffer<T>::FrameBuffer(int size) :
                    FrameBuffer<T>(new T[size], size) {
}

template<typename T>
FrameBuffer<T>::FrameBuffer(T *$data, int size)
                            : _data{$data}, _size{size} {
    _bpe = sizeof(T);
}

template<typename T>
FrameBuffer<T>::~FrameBuffer() {
    delete [] _data;
}

template<typename T>
int FrameBuffer<T>::size() {
    return _size;
}

template<typename T>
T &FrameBuffer<T>::readAt(int index) {
    return _data[index];
}

template<typename T>
void FrameBuffer<T>::writeAt(const T &value, int index) {
    _data[index] = value;
}

template<typename T>
const T &FrameBuffer<T>::operator()(int index) const {
    return _data[index];
}

template<typename T>
T *FrameBuffer<T>::data() {
    return _data;
}

template<typename T>
void FrameBuffer<T>::fill(const T &value) {
//    memset(_data, value, _size * sizeof(T));
//    memcpy(_data, value, _size * sizeof(T));
//    std::fill(_pixels, &_pixels[_width*_height], value);

    int size2 = _size;
    for (int ix = 0; ix < size2; ++ix) {
        _data[ix] = value;
    }
}


