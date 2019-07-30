//
// Created by Tomer Shalev on 2019-06-15.
//

#pragma once

#include "PixelFormat.h"
//#include <algorithm>

template<typename T>
class Buffer {
public:
    Buffer(int size);
    Buffer(T* $pixels, int size);
    ~Buffer();
    int size();
    T & readAt(int index);
    void writeAt(const T &value, int index);
    const T &operator()(int index) const;
    T * data();
    void fill(const T &value);

protected:
    int _size = 0;
    T *_data = nullptr;
};


#include "../../src/Buffer.tpp"