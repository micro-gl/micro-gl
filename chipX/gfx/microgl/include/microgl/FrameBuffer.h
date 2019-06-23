//
// Created by Tomer Shalev on 2019-06-15.
//

#pragma once

#include "PixelFormat.h"

template<typename T>
class FrameBuffer {
public:
    FrameBuffer(int size);
    FrameBuffer(T* $pixels, int size);
    ~FrameBuffer();
    int size();
    T & readAt(int index);
    void writeAt(const T &value, int index);
    const T &operator()(int index) const;
    T * data();
    void fill(const T &value);

protected:
    int _size = 0;
    T *_data = nullptr;
    uint8_t _bpe = sizeof(T);
};

#include "../src/FrameBuffer.tpp"