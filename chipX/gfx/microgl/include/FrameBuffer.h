//
// Created by Tomer Shalev on 2019-06-15.
//

#pragma once

#include "PixelFormat.h"

template<typename T>
class FrameBuffer {
public:
    FrameBuffer(int w, int h, PixelFormat format = PixelFormat::RGBA8888);
    ~FrameBuffer();
    int width();
    int height();
    T & readAt(int x, int y);
    int indexOf(int x, int y);
    void writeAt(const T &value, int x, int y);
    const T &operator()(int x, int y) const;
    T * pixels();
    void fill(const T value);
    PixelFormat format();


private:
    int _width = 0, _height = 0;
    PixelFormat _format = PixelFormat::RGBA8888;
    T *_pixels = nullptr;
};

typedef FrameBuffer<uint8_t> FrameBuffer8Bit;
typedef FrameBuffer<uint16_t> FrameBuffer16Bit;
typedef FrameBuffer<uint32_t> FrameBuffer32Bit;


#include "../src/FrameBuffer.tpp"