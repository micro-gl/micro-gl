#pragma once

#include <microgl/array_container.h>
#include <type_traits>

template<typename T>
class dynamic_array : public array_container<T> {
public:
    using index = unsigned int;

    dynamic_array(const std::initializer_list<T> &list) : dynamic_array(list.size()) {
        for(auto it = list.begin(); it!=list.end(); it++)
            this->push_back(*it);
    }

    dynamic_array(const dynamic_array<T> &container) : dynamic_array(container.size()) {
        for(auto ix = 0; ix < container.size(); ix++)
            this->push_back(container[ix]);
    }

    explicit dynamic_array(index capacity = 0) {
        _cap = capacity;

        if(_cap > 0)
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

    const T* data() const override {
        return _data;
    }

    dynamic_array<T> & operator=(const dynamic_array<T> &container) {
        this->clear();
        for(auto ix = 0; ix < container.size(); ix++)
            this->push_back(container[ix]);

        return (*this);
    }


    T& operator[](index i) override {
        return _data[i];
    }

    const T& operator[](index i) const override {
        return _data[i];
    }

    const T& peek() override {
        return (*this)[_current];
    }

    void alloc_(bool up) {
        _cap = up ? _cap<<1 : _cap>>1;
        T* _new = nullptr;

        if(_cap==0 && up)
            _cap = 1;

        if(_cap>0) {
            _new = new T[_cap];

            for (index ix = 0; ix < size(); ++ix) {
                _new[ix] = _data[ix];
            }

        }

        if(_data)
            delete [] _data;

        _data = _new;
    }

    void push_back(const T & v) override {
        if(int(_current)>int(_cap-1))
            alloc_(true);

        _data[_current++] = v;
    }

    void push_back(const array_container<T> & container) override {
        const int count = container.size();
        for (int ix = 0; ix < count; ++ix) {
            this->push_back(container[ix]);
        }
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

    T& back() {
        return _data[_current-1];
    }

    bool empty() {
        return _current==0;
    }

    index size() const override {
        return _current;
    }

    index capacity() const override {
        return _cap;
    }

private:
    T *_data = nullptr;
    index _current = 0u;
    index _cap = 0u;
};
