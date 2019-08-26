
#pragma once

template<typename T>
class DynamicArray {
public:
    using index = unsigned int;

    explicit DynamicArray(index capacity = 10) {
        _cap = capacity;
        _data = new T[_cap];
    }

    ~DynamicArray() {
        delete [] _data;
        _data = nullptr;
        _cap = 0;
        _current = 0;
    }

    T* getData() {
        return _data;
    }

    const T& operator[](index i) {
      return _data[i];
    }

    const T& peek() {
        return *this[_current];
    }

    void alloc_(bool up) {
        _cap = up ? _cap<<1 : _cap>>1;
        T* _new = new T[_cap];

        for (int ix = 0; ix < size(); ++ix) {
            _new[ix] = _data[ix];
        }

        delete [] _data;

        _data = _new;
    }

    void push_back(const T & v) {
        if(_current==_cap-1)
            alloc_(true);

        _data[_current++] = v;
    }

    void pop_back() {
        if(_current < (_cap>>1))
            alloc_(false);

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
        return _cap;
    }

private:
    T *_data = nullptr;
    index _current = 0;
    index _cap = 0;
};
