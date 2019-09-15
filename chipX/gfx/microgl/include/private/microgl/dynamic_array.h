#pragma once

#include <microgl/array_container.h>

template<typename T>
class dynamic_array : public array_container<T> {
public:
    using index = unsigned int;

    explicit dynamic_array(index capacity = 10) {
        _cap = capacity;
        _data = new T[_cap];
    }

    ~dynamic_array() {
        delete [] _data;
        _data = nullptr;
        _cap = 0;
        _current = 0;
    }

    T* data() override {
        return _data;
    }

    T& operator[](index i) override {
      return _data[i];
    }

    const T& peek() override {
        return (*this)[_current];
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

    void push_back(const T & v) override {
        if(_current==_cap-1)
            alloc_(true);

        _data[_current++] = v;
    }

    void pop_back() override {
        if(_current < (_cap>>1))
            alloc_(false);

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
        return _cap;
    }

private:
    T *_data = nullptr;
    index _current = 0;
    index _cap = 0;
};
