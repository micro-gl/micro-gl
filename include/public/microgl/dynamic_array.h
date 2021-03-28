#pragma once

namespace dynamic_array_traits {

    template< class T > struct remove_reference      {typedef T type;};
    template< class T > struct remove_reference<T&>  {typedef T type;};
    template< class T > struct remove_reference<T&&> {typedef T type;};

    template <class _Tp>
    inline
    typename remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    {
        typedef typename remove_reference<_Tp>::type _Up;
        return static_cast<_Up&&>(__t);
    }

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

    inline
    void * memcpy(void *dst, const void *src, const unsigned len) noexcept
     {
         if ((unsigned long)dst % sizeof(long) == 0 &&
             (unsigned long)src % sizeof(long) == 0 &&
             len % sizeof(long) == 0) {
             long *d = (long *)dst;
             const long *s = (const long *)src;
             for (int i=0; i<len/sizeof(long); i++)
                 *d++ = *s++;
         }
         else {
             char *d = (char *)dst;
             const char *s = (char *)src;
             for (int i=0; i<len; i++)
                 *d++ = *s++;
         }

         return dst;
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

    explicit dynamic_array(unsigned capacity = 0) noexcept : _cap{capacity} {
        if(_cap > 0) _data = new T[_cap];
    }

    explicit dynamic_array(signed capacity) noexcept : dynamic_array{unsigned(capacity)} {
    }

    ~dynamic_array() noexcept { drain(); }

    dynamic_array<T> & operator=(const dynamic_array<T> &container) noexcept {
        if(this!= &container) {
            clear();
            for(index ix = 0; ix < container.size(); ix++)
                push_back(container[ix]);
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
        const auto old_size = _current;
        const auto new_size = up ? (_cap==0?1:_cap*2) : _cap/2;
        const auto copy_size = old_size<new_size ? old_size : new_size;
        T* _new = new T[new_size];
//        for (index ix = 0; ix < copy_size; ++ix)
//            _new[ix] = dynamic_array_traits::move(_data[ix]);
        // this reduces binary size
        dynamic_array_traits::memcpy(_new, _data, copy_size*sizeof (T));
        delete [] _data;
        _data = reinterpret_cast<T*>(_new);
        _cap = new_size;
    }

    void push_back(const T & v) noexcept {
        if(int(_current)>int(_cap-1)) {
//             copy the value, edge case if v belongs
//             to the dynamic array
            const T vv = v;
            alloc_(true);
            _data[_current++] = vv;
        } else {
            _data[_current++] = v;
        }
    }

    void push_back(T && v) noexcept {
        if(int(_current)>int(_cap-1)) {
            // copy the value, edge case if v belongs
            // to the dynamic array
            const T vv = dynamic_array_traits::move(v);
            alloc_(true);
            _data[_current++] = dynamic_array_traits::move(vv);
        } else {
            _data[_current++] = dynamic_array_traits::move(v);
        }
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

    void clear() noexcept {
        for (int ix = 0; ix < capacity(); ++ix)
            _data[ix].~T();
        _current = 0;
    }
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
