#pragma once

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

    T *_data = nullptr;

protected:
    int _size = 0;
    uint8_t _bpe = sizeof(T);
};

#include "../../src/FrameBuffer.tpp"