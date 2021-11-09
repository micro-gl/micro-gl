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

namespace buffer_traits {
    template< class T > struct remove_reference      {typedef T type;};
    template< class T > struct remove_reference<T&>  {typedef T type;};
    template< class T > struct remove_reference<T&&> {typedef T type;};

    template <class _Tp> inline typename remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept {
        typedef typename remove_reference<_Tp>::type _Up;
        return static_cast<_Up&&>(__t);
    }

    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type& __t) noexcept
    { return static_cast<_Tp&&>(__t); }

    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type&& __t) noexcept
    { return static_cast<_Tp&&>(__t); }
}

template<typename element_type, class allocator_type>
class buffer {
private:
    using rebind_alloc = typename allocator_type::template rebind<element_type>::other;

    rebind_alloc _allocator;
    element_type *_data = nullptr;
    bool owner = false;
    int _size = 0;

    static element_type * allocate_and_construct(const int size, rebind_alloc & allocator) {
        auto * mem = allocator.allocate(size);
        for (int ix = 0; ix < size; ++ix)
            new (mem+ix) element_type();
        return (element_type *)mem;
    }

    void copy_from(const buffer & val) {
        destroyIfPossibleAndReset();
        _data = allocate_and_construct(val.size(), _allocator); //new element_type[val.size()];
        _size = val.size();
        owner = true;
        for (int ix = 0; ix < _size; ++ix)
            _data[ix]=val._data[ix];
    }

    void move_from(buffer & val) {
        const bool is_same_allocator = _allocator==val._allocator;
        if(is_same_allocator) {
            // same allocator then do classic fast move
            destroyIfPossibleAndReset();
            _data = val.data();
            _size = val.size();
            owner = val.owner;
            val.owner = false;
        } else {
            // NOT same allocator then move item by item.
            destroyIfPossibleAndReset();
            _data = allocate_and_construct(val.size(), _allocator); //new element_type[val.size()];
            _size = val.size();
            owner = true;
            for (int ix = 0; ix < _size; ++ix)
                _data[ix]=buffer_traits::move(val._data[ix]);
        }
        val.destroyIfPossibleAndReset();
    }

public:
    explicit buffer(int size, const allocator_type & allocator) :
            _size{size}, owner{true}, _allocator(allocator) {
        _data = allocate_and_construct(size, _allocator);
    }
    buffer(element_type* $data, int size,
           const allocator_type & allocator=allocator_type()) :
                    _data{$data}, _size{size}, owner{false}, _allocator(allocator) {}
    buffer(const buffer & val) : _allocator(val._allocator) { copy_from(val); }
    buffer(buffer && val) noexcept : _allocator(val._allocator) { move_from(val); }
    ~buffer() { destroyIfPossibleAndReset(); }

    buffer & operator=(const buffer & val) {
        if(&val==this) return *this;
        copy_from(val);
        return *this;
    }
    buffer & operator=(buffer && val) noexcept { move_from(val); return *this; }

    void destroyIfPossibleAndReset() {
        if(owner) {
            for (int ix = 0; ix < size(); ++ix)
                _data[ix].~element_type();
            _allocator.deallocate(_data, _size);
        }
        _data=nullptr;
        _size=0;
    };
    int size()  const { return _size; }
    element_type & readAt(int index) { return _data[index]; }
    void writeAt(const element_type &value, int index) { _data[index] = value; }
    const element_type &operator[](int index) const { return _data[index]; }
    element_type &operator[](int index) { return _data[index]; }
    element_type * data() { return _data; }
    const element_type * data() const { return _data; }
    void fill(const element_type &value) {
        int size2 = _size;
        for (int ix = 0; ix < size2; ++ix)
            _data[ix] = value;
    }
};