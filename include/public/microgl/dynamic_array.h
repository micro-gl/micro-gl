#pragma once

namespace dynamic_array_traits {

    template< class T > struct remove_reference      {typedef T type;};
    template< class T > struct remove_reference<T&>  {typedef T type;};
    template< class T > struct remove_reference<T&&> {typedef T type;};

    template <class _Tp>
    inline
    _Tp&&
    forward(typename remove_reference<_Tp>::type& __t) noexcept
    {
        return static_cast<_Tp&&>(__t);
    }

    template <class _Tp>
    inline
    _Tp&&
    forward(typename remove_reference<_Tp>::type&& __t) noexcept
    {
        return static_cast<_Tp&&>(__t);
    }

}

template<typename T>
class dynamic_array {
    using const_dynamic_array_ref = const dynamic_array<T> &;
public:
    using index = unsigned int;
    using type = T;

private:
    T *_data = nullptr;
//    T *_data = new T[800];
    index _current = 0u;
    index _cap = 0u;

public:
    template<class Iterable>
    dynamic_array(const Iterable &list) noexcept : dynamic_array(index(list.size())) {
        for (const auto & item : list)
            push_back(item);
    }

    dynamic_array(const dynamic_array<T> &container) noexcept : dynamic_array(container.size()) {
        for(index ix = 0; ix < container.size(); ix++)
            this->push_back(container[ix]);
    }

    dynamic_array(dynamic_array<T> && container)  noexcept {
        _data = container._data;
        _current = container._current;
        _cap = container._cap;
        container._data=nullptr;
        container.clear();
    }

    explicit dynamic_array(unsigned capacity = 0) noexcept {
        _cap = capacity;
        if(_cap > 0) _data = new T[_cap];
    }

    explicit dynamic_array(signed capacity) noexcept : dynamic_array{unsigned(capacity)} {
    }

    ~dynamic_array() noexcept { drain(); }

    dynamic_array<T> & operator=(const dynamic_array<T> &container) noexcept {
        if(this!= &container) {
            this->clear();
            for(index ix = 0; ix < container.size(); ix++)
                this->push_back(container[ix]);
        }
        return (*this);
    }

    dynamic_array<T> & operator=(dynamic_array<T> &&container) noexcept {
        if(_data) delete [] _data;
        _data = container._data;
        _current = container._current;
        _cap = container._cap;
        container._data=nullptr;
        container.clear();
        return (*this);
    }

    T& operator[](index i) noexcept { return _data[i]; }
    const T& operator[](index i) const noexcept { return _data[i]; }
    const T& peek() noexcept { return (*this)[_current]; }

    void alloc_(bool up) noexcept {
        _cap = up ? _cap<<1 : _cap>>1;
        T* _new = nullptr;

        if(_cap==0 && up)
            _cap = 1;

        if(_cap>0) {
            _new = new T[_cap];
            for (index ix = 0; ix < size(); ++ix)
                _new[ix] = _data[ix];
        }

        if(_data) delete [] _data;

        _data = _new;
    }

    int push_back(const T & v) noexcept {
        if(int(_current)>int(_cap-1)) {
            // copy the value, edge case if v belongs
            // to the dynamic array
            const T vv = v;
            alloc_(true);
            _data[_current++] = vv;
        } else {
            _data[_current++] = v;
        }
        return _current-1;
    }

    template<typename... ARGS>
    int emplace_back(ARGS&&... args) noexcept {
        if(int(_current)>int(_cap-1))
            alloc_(true);
        auto * mem_loc = &_data[_current++];
        new (mem_loc) T(dynamic_array_traits::forward<ARGS>(args)...);
        return _current-1;
    }

    void push_back(const_dynamic_array_ref container) noexcept {
        const int count = container.size();
        for (int ix = 0; ix < count; ++ix)
            this->push_back(container[ix]);
    }

    void pop_back() noexcept {
        if(_current < (_cap>>1)) alloc_(false);
        if(_current==0) return;
        _data[_current--].~T();
    }

    void move(index idx) noexcept {
        if(idx < capacity())
            _current = idx;
    }

    void drain() noexcept {
        delete [] _data;
        _data = nullptr;
        _cap = 0;
        _current = 0;
    }

    void clear() noexcept { _current = 0; }
    T* data() noexcept { return _data; }
    const T* data() const noexcept { return _data; }
    T& back() noexcept { return _data[_current-1]; }
    bool empty() noexcept { return _current==0; }
    index size() const noexcept { return _current; }
    index capacity() const noexcept { return _cap; }
    const T* begin() const noexcept {return _data;}
    const T* end() const noexcept {return _data + size();}
    T* begin() noexcept {return _data;}
    T* end()  noexcept {return _data + size();}

};
