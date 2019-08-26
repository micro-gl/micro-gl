
#pragma once

template<typename T, unsigned N>
class StaticArray {
public:
    using index = unsigned int;

    explicit StaticArray() = default;

    const T& operator[](index i) {
        return _data[i];
    }

    T* getData() {
        return _data;
    }

    const T& peek() {
        return *this[_current];
    }

    void push_back(const T & v) {
        if(_current==N-1)
            return;
        _data[_current++] = v;
    }

    void pop_back() {
        if(_current==0)
            return;

        _data[_current--].~T();
    }

    void clear() {
        _current = 0;
    }

    index size() {
        return _current;
    }

    index capacity() {
        return N;
    }

private:
    T _data[N];
    index _current = 0;
};
