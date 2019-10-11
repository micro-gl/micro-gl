#pragma once

#include <microgl/array_container.h>

template<typename T, unsigned N>
class static_array : public array_container<T> {
public:
    using index = unsigned int;

    explicit static_array() = default;
    static_array(int dummy){}

    static_array(const std::initializer_list<T> &list) : static_array() {
        for(auto it = list.begin(); it!=list.end(); it++)
            this->push_back(*it);
    }

    static_array(const static_array &container) : static_array() {
        for(auto ix = 0; ix < container.size(); ix++)
            this->push_back(container[ix]);
    }

    static_array & operator=(const static_array&container) {
        this->clear();
        for(index ix = 0; ix < container.size(); ix++)
            this->push_back(container[ix]);

        return (*this);
    }

    T& operator[](index i) override {
        return _data[i];
    }

    const T& operator[](index i) const override {
        return _data[i];
    }

    T* data() override {
        return _data;
    }

    const T* data() const override {
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

    void push_back(const array_container<T> & container) override {
        for (index ix = 0; ix < container.size(); ++ix) {
            this->push_back(container[ix]);
        }
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

    index size() const override {
        return _current;
    }

    index capacity() const override {
        return N;
    }

private:
    T _data[N];
    index _current = 0;
};
