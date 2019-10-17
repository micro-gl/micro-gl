#pragma once

//#include <microgl/array_container.h>

template<typename T, unsigned N>
class static_array {
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

    T& operator[](index i) {
        return _data[i];
    }

    const T& operator[](index i) const {
        return _data[i];
    }

    T* data() {
        return _data;
    }

    const T* data() const {
        return _data;
    }

    const T& peek() {
        return (*this)[_current];
    }

    void push_back(const T & v) {
        if(_current==N-1)
            return;
        _data[_current++] = v;
    }

    void push_back(const static_array<T> & container) {
        for (index ix = 0; ix < container.size(); ++ix) {
            this->push_back(container[ix]);
        }
    }

    void push_back(static_array<T> & container) {
        for (index ix = 0; ix < container.size(); ++ix) {
            this->push_back(container[ix]);
        }
    }

    void pop_back() {
        if(_current==0)
            return;

        _data[_current--].~T();
    }

    void move(index idx) {
        if(idx < capacity())
            _current = idx;
    }

    void clear() {
        _current = 0;
    }

    index size() const {
        return _current;
    }

    index capacity() const {
        return N;
    }

private:
    T _data[N];
    index _current = 0;
};
