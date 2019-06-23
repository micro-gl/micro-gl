
#include "../include/microgl/Buffer.h"

template<typename T>
Buffer<T>::Buffer(int size) :
        Buffer<T>(new T[size], size) {
}

template<typename T>
Buffer<T>::Buffer(T *$data, int size)
                            : _data{$data}, _size{size} {

}

template<typename T>
Buffer<T>::~Buffer() {
    delete [] _data;
}

template<typename T>
int Buffer<T>::size() {
    return _size;
}

template<typename T>
T &Buffer<T>::readAt(int index) {
    return _data[index];
}

template<typename T>
void Buffer<T>::writeAt(const T &value, int index) {
    _data[index] = value;
}

template<typename T>
const T &Buffer<T>::operator()(int index) const {
    return readAt(index);
}

template<typename T>
T *Buffer<T>::data() {
    return _data;
}

template<typename T>
void Buffer<T>::fill(const T &value) {
    memset(_data, value, _size* sizeof(T));
//    std::fill(_pixels, &_pixels[_width*_height], value);
}

