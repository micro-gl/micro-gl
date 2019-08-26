#pragma once

#include <microgl/ArrayContainer.h>

template<typename T, unsigned N>
class StaticArray : public ArrayContainer<T> {
public:
    using index = unsigned int;

    explicit StaticArray() = default;

    const T& operator[](index i) override {
        return _data[i];
    }

    T* getData() override {
        return _data;
    }

    const T& peek() override {
        return (*this)[_current];
    }

    void push_back(const T & v) override {
        if(_current==N-1)
            return;
        _data[_current++] = v;
    }

    void pop_back() override {
        if(_current==0)
            return;

        _data[_current--].~T();
    }

    void clear() override {
        _current = 0;
    }

    index size() override {
        return _current;
    }

    index capacity() override {
        return N;
    }

private:
    T _data[N];
    index _current = 0;
};
