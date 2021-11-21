/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

namespace dynamic_array_traits {

    template< class T > struct remove_reference      {typedef T type;};
    template< class T > struct remove_reference<T&>  {typedef T type;};
    template< class T > struct remove_reference<T&&> {typedef T type;};

    template <class _Tp>
    inline typename remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    {
        typedef typename remove_reference<_Tp>::type _Up;
        return static_cast<_Up&&>(__t);
    }

    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type& __t) noexcept
    {
        return static_cast<_Tp&&>(__t);
    }

    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type&& __t) noexcept
    {
        return static_cast<_Tp&&>(__t);
    }

    /**
     * standard allocator
     * @tparam T the allocated object type
     */
    template<typename T>
    class std_allocator {
    public:
        using value_type = T;
        using size_t = unsigned long;
    public:
        template<class U>
        explicit std_allocator(const std_allocator<U> & other) noexcept { };
        explicit std_allocator()=default;

        template <class U, class... Args>
        void construct(U* p, Args&&... args) {
            new(p) U(dynamic_array_traits::forward<Args>(args)...);
        }

        T * allocate(size_t n) { return (T *)operator new(n * sizeof(T)); }
        void deallocate(T * p, size_t n=0) { operator delete (p); }

        template<class U> struct rebind {
            typedef std_allocator<U> other;
        };
    };

    template<class T1, class T2>
    bool operator==(const std_allocator<T1>& lhs, const std_allocator<T2>& rhs ) noexcept {
        return true;
    }
}

/**
 * Minimal vector like container, does not obey all of the propagate syntax that
 * Allocator Aware Container follows
 * @tparam T the type
 * @tparam Alloc the allocator type
 */
template<typename T, class Alloc=dynamic_array_traits::std_allocator<T>>
class dynamic_array {
    using const_dynamic_array_ref = const dynamic_array<T, Alloc> &;
public:
    using value_type = T;
    using allocator_type = Alloc;
    using index = unsigned int;
    using type = T;
    using uint = unsigned int;

private:
    using rebind_allocator_type = typename Alloc::template rebind<value_type>::other;

    T *_data = nullptr;
    rebind_allocator_type _alloc;
    index _current = 0u;
    index _cap = 0u;

public:
    explicit dynamic_array(const Alloc & alloc = Alloc()) noexcept : _alloc{alloc} {
    }

    dynamic_array(const uint count, const T & value = T(), const Alloc & alloc = Alloc()) :
            dynamic_array(alloc) {
        reserve(count);
        for (int ix = 0; ix < count; ++ix) push_back(value);
    }

    template<class Iterable>
    dynamic_array(const Iterable &list, const Alloc & alloc= Alloc()) noexcept :
            dynamic_array(alloc) {
        reserve(list.size());
        for (const auto & item : list) push_back(item);
    }

    dynamic_array(const dynamic_array & other, const Alloc & alloc) noexcept :
            dynamic_array(alloc) {
        reserve(other.size());
        for (const auto & item : other) push_back(item);
    }

    dynamic_array(const dynamic_array & other) noexcept :
            dynamic_array(other, other.get_allocator()) {
    }

    dynamic_array(dynamic_array && other, const Alloc & alloc) noexcept :
            dynamic_array(alloc) {
        reserve(other.size());
        for (const auto & item : other) push_back(dynamic_array_traits::move(item));
    }

    dynamic_array(dynamic_array && other) noexcept : dynamic_array{other.get_allocator()} {
        _data = other._data;
        _current = other._current;
        _cap = other._cap;
        other._data=nullptr;
        other._cap=0;
        other._current=0;
    }

    ~dynamic_array() noexcept { drain(); }

    void reserve(uint new_cap) {
        bool avoid = new_cap <= capacity();
        if(avoid) return;
        T * _new_data = _alloc.allocate(new_cap);
        // construct default objects for validity
        for (int ix = 0; ix < new_cap; ++ix)
            new (_new_data + ix) T();
        // move old objects
        for (int ix = 0; ix < size(); ++ix)
            _new_data[ix] = dynamic_array_traits::move(_data[ix]);
        _cap = new_cap;
        // no need to destruct because the items were moved
        if(_data!=nullptr) _alloc.deallocate(_data);
        _data = _new_data;
    }

    dynamic_array & operator=(const dynamic_array & other) noexcept {
        if(this!= &other) {
            clear();
            reserve(other.size());
            for(index ix = 0; ix < other.size(); ix++)
                push_back(other[ix]);
        }
        return (*this);
    }

    dynamic_array & operator=(dynamic_array && other) noexcept {
        // two cases:
        // 1. if the allocators are equal, then move the data completely.
        // 2. otherwise, move element by element
        const bool are_equal = _alloc == other.get_allocator();
        const bool self_assign = this == &other;
        if(self_assign) return *this;

        if(are_equal) {
            // clear and destruct current elements
            clear();
            // deallocate the current memory
            _alloc.deallocate(_data, capacity());
            // move everything from other
            _data = other._data;
            other._data=nullptr;
            other._cap=0;
            other._current=0;
        } else {
            if(other.size() >= capacity()) {
                clear(); // clear and destruct current objects
                _alloc.deallocate(_data, capacity()); // de allocate the chunk
                _data = _alloc.allocate(other.size()); // create new chunk
                // construct default objects for validity
                for (int ix = 0; ix < other.size(); ++ix)
                    new (_data + ix) T();
            }
            // move other items into current memory
            for (int ix = 0; ix < other.size(); ++ix)
                _data[ix] = dynamic_array_traits::move(other[ix]);
            for (int ix = other.size(); ix < capacity(); ++ix)
                _data[ix] = T();
            // no need to destruct other's items
            other._current=0;
        }

        _current = other._current;
        _cap = other._cap;

        return (*this);
    }

    T& operator[](index i) noexcept { return _data[i]; }
    const T& operator[](index i) const noexcept { return _data[i]; }
    const T& peek() noexcept { return (*this)[_current]; }

    void alloc_(bool up) noexcept {
        const auto old_size = _current;
        const auto new_size = up ? (_cap==0?1:_cap*2) : _cap/2;
        const auto copy_size = old_size<new_size ? old_size : new_size;

        T * _new_data = _alloc.allocate(new_size);
        // construct default objects for validity
        for (int ix = 0; ix < new_size; ++ix)
            new (_new_data + ix) T();
        // move all previous objects into new location,
        // therefore we do not need to destruct because we move
        for (index ix = 0; ix < copy_size; ++ix)
            _new_data[ix] = dynamic_array_traits::move(_data[ix]);

        // de allocate old data
        _alloc.deallocate(_data, capacity());
        _data = reinterpret_cast<T*>(_new_data);
        _cap = new_size;
    }

    void push_back(const T & v) noexcept {
        if(int(_current)>int(_cap-1)) {
            // copy the value, edge case if v belongs
            // to the dynamic array
            const T vv = v;
            alloc_(true);
            _data[_current++] = vv;
        } else _data[_current++] = v;
    }

    void push_back(T && v) noexcept {
        if(int(_current)>int(_cap-1)) {
            // copy the value, edge case if v belongs
            // to the dynamic array
            const T vv = dynamic_array_traits::move(v);
            alloc_(true);
            _data[_current++] = dynamic_array_traits::move(vv);
        } else _data[_current++] = dynamic_array_traits::move(v);
    }

    template<typename... ARGS>
    int emplace_back(ARGS&&... args) noexcept {
        if(int(_current)>int(_cap-1)) alloc_(true);
        auto * mem_loc = &_data[_current++];
        new (mem_loc) T(dynamic_array_traits::forward<ARGS>(args)...);
        return _current-1;
    }

    void push_back(const_dynamic_array_ref container) noexcept {
        const int count = container.size();
        for (int ix = 0; ix < count; ++ix) this->push_back(container[ix]);
    }

    void pop_back() noexcept {
        if(_current < (_cap>>1)) alloc_(false);
        if(_current==0) return;
        _data[_current--].~T();
    }

    void move(index idx) noexcept {
        if(idx < capacity()) _current = idx;
    }

    void drain() noexcept {
        clear();
        if(_data!=nullptr)
            _alloc.deallocate(_data, capacity());
        _data = nullptr;
        _cap = 0;
        _current = 0;
    }

    void clear() noexcept {
        for (int ix = 0; ix < capacity(); ++ix) _data[ix].~T();
        _current = 0;
    }
    Alloc get_allocator() const noexcept { return Alloc(_alloc); }
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