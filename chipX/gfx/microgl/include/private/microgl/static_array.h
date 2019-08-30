#pragma once

#include <microgl/array_container.h>

template<typename T, unsigned N>
class static_array : public array_container<T> {
public:
    using index = unsigned int;

    explicit static_array() = default;

    T& operator[](index i) override {
        return _data[i];
    }

    T* data() override {
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

    void move(index idx) override {
        if(idx < capacity())
            _current = idx;
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
