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

#include "traits.h"

/**
 * Minimal vector like container, does not obey all of the propagate syntax that
 * Allocator Aware Container follows
 * @tparam T the type
 * @tparam Alloc the allocator type
 */
template<typename T, class Alloc=microtess::std_allocator<T>>
class dynamic_array {
public:
    using value_type = T;
    using size_type = microtess::size_t;
    using allocator_type = Alloc;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    struct out_of_bounds_exception {};

private:
    using rebind_allocator_type = typename Alloc::template rebind<value_type>::other;

    void throw_out_of_bounds_exception_if_can() {
#ifdef MICRO_CONTAINERS_ENABLE_THROW
        throw out_of_bounds_exception();
#endif
    }

    T *_data;
    rebind_allocator_type _alloc;
    size_type _current;
    size_type _cap;

public:
    explicit dynamic_array(const Alloc & alloc) noexcept :
        _data(nullptr), _current(0), _cap(0), _alloc(alloc) {}
    dynamic_array() noexcept : dynamic_array(Alloc()) {}
    dynamic_array(const size_type count, const T & value, const Alloc & alloc = Alloc()) :
            dynamic_array(alloc) {
        reserve(count);
        for (size_type ix = 0; ix < count; ++ix) push_back(value);
    }
    explicit dynamic_array(size_type count, const Alloc& alloc = Alloc()) :
                        dynamic_array(count, T(), alloc) {}
    template<class Iterable>
    dynamic_array(const Iterable &list, const Alloc & alloc= Alloc()) :
            dynamic_array(list.begin(), list.end(), alloc) {}
    template<class InputIt, typename bb = microtess::traits::enable_if_t<!microtess::traits::is_integral<InputIt>::value>>
    dynamic_array(InputIt first, InputIt last, const Alloc & alloc= Alloc()) :
            dynamic_array(alloc) {
        reserve(last-first);
        while(first!=last) { push_back(*first); ++first; }
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
        for (auto & item : other) push_back(microtess::traits::move(item));
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

    // Element access
    reference operator[](size_type i) { return _data[i]; }
    const_reference operator[](size_type i) const { return _data[i]; }
    reference at(size_type pos) {
        if(pos>=0 && pos < size()) return _data[pos];
        throw_out_of_bounds_exception_if_can();
    }
    const_reference at(size_type pos) const {
        if(pos>=0 && pos < size()) return _data[pos];
        throw_out_of_bounds_exception_if_can();
    }
    T* data() noexcept { return _data; }
    const T* data() const noexcept { return _data; }
    reference back() { return _data[_current-1]; }
    reference front() { return _data[0]; }
    const_reference back() const { return _data[_current-1]; }
    const_reference front() const { return _data[0]; }

    // Capacity
    bool empty() noexcept { return _current==0; }
    size_type size() const noexcept { return _current; }
    size_type capacity() const noexcept { return _cap; }
    void shrink_to_fit() { re_alloc_to(size()); }
    void reserve(size_type new_cap) {
        if(new_cap <= capacity()) return;
        re_alloc_to(new_cap);
    }

    // Assignment operator
    dynamic_array & operator=(const dynamic_array & other) noexcept {
        if(this!= &other) {
            clear();
            reserve(other.size());
            for(size_type ix = 0; ix < other.size(); ix++)
                push_back(other[ix]);
        }
        return (*this);
    }

    dynamic_array & operator=(dynamic_array && other) noexcept {
        // two cases:
        // 1. if the allocators are equal, then move the data completely.
        // 2. otherwise, move element by element
        const bool are_equal = _alloc == other.get_allocator();
        if(this==&other) return *this;
        clear();
        if(are_equal) {
            // clear and destruct current elements
            // deallocate the current memory
            _alloc.deallocate(_data, capacity());
            // move everything from other
            _data = other._data;
            _current = other._current;
            _cap = other._cap;
            // no need to de-allocate because we stole the memory
            other._data=nullptr;
            other._cap=0;
            other._current=0;
        } else {
            if(other.size() >= capacity()) {
                _alloc.deallocate(_data, capacity()); // de allocate the chunk
                _data = _alloc.allocate(other.capacity()); // create new chunk
            }
            // move other items into current memory
            for (size_type ix = 0; ix < other.size(); ++ix)
                ::new (_data + ix, microtess_new::blah) T(microtess::traits::move(other[ix]));
            // no need to destruct other's items
            _current = other._current;
            _cap = other._cap;
            // different allocators, we do not de-allocate the other array.
        }
        return (*this);
    }

    // Modifiers

private:
    void alloc_(bool up) noexcept {
        const auto new_cap = up ? (_cap==0?1:_cap*2) : _cap/2;
        re_alloc_to(new_cap);
    }
    void re_alloc_to(size_type new_capacity) noexcept {
        // warning, if allocating down, _current/size() becomes dangling, so
        // you have to take care of it externally.
        // re-alloc to another capacity and move old elements
        const auto old_size = _current;
        const auto copy_size = old_size < new_capacity ? old_size : new_capacity;

        T * _new_data = nullptr;
        if(new_capacity) {
            _new_data = _alloc.allocate(new_capacity);
            // move all previous objects into new location,
            // therefore we do not need to destruct because we move from same allocator
            for (size_type ix = 0; ix < copy_size; ++ix)
                ::new(_new_data + ix, microtess_new::blah) T(microtess::traits::move(_data[ix]));
        }
        if(_data) { // de allocate old data
            // destruct suffix items in old memory, that didn't make it to new memory,
            // This happens when shrinking is happening.
            for (size_type ix = copy_size; ix < old_size; ++ix) (_data + ix)->~T();
            _alloc.deallocate(_data, capacity());
        }
        _data = _new_data;
        _cap = new_capacity;
    }
    template<class TT>
    void internal_push_back(TT && v) noexcept {
        if(size() >= capacity()) {
            // copy the value, edge case if v belongs
            // to the dynamic array
            auto vv = microtess::traits::forward<TT>(v); // copy-or-move ctor
            alloc_(true);
            ::new(_data + _current++, microtess_new::blah) T(microtess::traits::move(vv));
        } else ::new(_data + _current++, microtess_new::blah) T(microtess::traits::forward<TT>(v));;
    }
    template<class TT>
    void internal_resize(size_type count, TT && perfect_forwarded_value) {
        const size_type curr_size = size();
        if(count==curr_size) return;
        if(count < curr_size) { // reduce size
            const auto delta = curr_size-count;
            for (size_type ix = 0; ix < delta; ++ix) pop_back();
        } else { // increase capacity and fill extra items
            const auto delta = count-curr_size;
            reserve(curr_size+delta);
            for (size_type ix = 0; ix < delta; ++ix)
                push_back(microtess::traits::forward<TT>(perfect_forwarded_value));
        }
    }

public:
    void push_back(const T & v) noexcept { internal_push_back(v); }
    void push_back(T && v) noexcept {
        internal_push_back(microtess::traits::move(v));
    }
    void pop_back() {
        if(size()==0) return;
        _data[_current--].~T();
        if(size()==0 or (size()<(capacity()>>1))) alloc_(false);
    }

    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        return insert(pos, T(microtess::traits::forward<Args>(args)...));
    }
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        if(_current + 1> _cap) alloc_(true);
        ::new (_data + _current++, microtess_new::blah) T(microtess::traits::forward<Args>(args)...);
        return back();
    }

    void clear() noexcept {
        for (size_type ix = 0; ix < size(); ++ix) _data[ix].~T();
        _current = 0;
    }

    void drain() noexcept {
        clear();
        if(_data) _alloc.deallocate(_data, capacity());
        _data = nullptr; _cap = _current = 0;
    }

    // avoid overload resolution if InputIt is integral type, this can conflict with the
    // insert(const_iterator pos, size_type count, const T& value) overload for integral types
    template<class InputIt, typename bb = microtess::traits::enable_if_t<!microtess::traits::is_integral<InputIt>::value>>
    iterator insert(const_iterator pos, InputIt first, InputIt last, bool move=false) {
        if(pos>end() or pos<begin()) {
            throw_out_of_bounds_exception_if_can();
            return end();
        }
        if(first==last) return const_cast<iterator>(pos);
        // insert range before pos
        const auto dist_to_first = pos-begin();
        const auto needed_extra_size = last-first;
        const auto how_many_to_shift_right = end()-pos;
        const auto how_many_to_construct = end()-pos;
        const bool requires_expend = size()+needed_extra_size > capacity();
        if(requires_expend) {
            re_alloc_to(capacity() + needed_extra_size + 1);
            // latest pos is no longer valid, memory might have changed
            pos = begin()+dist_to_first;
        }
        auto current_end = end();
        auto new_end = current_end + needed_extra_size;
        //
#define minnnn(a,b) ((a)<(b) ? (a) : (b))
        const auto how_many_veterans_require_move_construction =
                minnnn(how_many_to_shift_right, needed_extra_size);
        const auto how_many_veterans_require_move_assignment =
                how_many_to_shift_right-how_many_veterans_require_move_construction;
        const auto how_many_range_require_copy_construction =
                needed_extra_size-how_many_veterans_require_move_construction;
#undef minnnn
        // move-construct at end the last elements, that belong to this
        for (size_type ix = 0; ix<how_many_veterans_require_move_construction; ++ix) { // count from [N..1]
            ::new (--new_end, microtess_new::blah) T(microtess::traits::move(*(--current_end)));
        } // move-assign at end the last elements, that belong to this
        for (size_type ix = 0; ix<how_many_veterans_require_move_assignment; ++ix) { // count from [N..1]
            *(--new_end) = microtess::traits::move(*(--current_end));
        }
        // copy-construct some range elements
        for (size_type ix = 0; ix<how_many_range_require_copy_construction; ++ix) { // count from [N..1]
            if(!move) ::new (--new_end, microtess_new::blah) T(*(--last));
            else ::new (--new_end, microtess_new::blah) T(microtess::traits::move(*(--last)));
        }
        do { // move-or-copy assign remaining items from iterator
            *(--new_end) = !move ? *(--last) : microtess::traits::move(*(--last));
        } while(last!=first);
        _current += needed_extra_size;
        return const_cast<iterator>(pos);
    }
    iterator insert(const_iterator pos, const T& value) {
        if(pos>end() or pos<begin()) {
            throw_out_of_bounds_exception_if_can();
            return end();
        }
        if(pos==end()) { push_back(value); return end(); }
        return insert<const_iterator>(pos, &value, (&value)+1);
    }
    iterator insert(const_iterator pos, T && value) {
        if(pos>end() or pos<begin()) {
            throw_out_of_bounds_exception_if_can();
            return end();
        }
        if(pos==end()) { push_back(microtess::traits::move(value)); return end(); }
        return insert<const_iterator>(pos, &value, (&value)+1, true);
    }
    iterator insert(const_iterator pos, size_type count, const T& value) {
        if(pos>end() or pos<begin()) {
            throw_out_of_bounds_exception_if_can();
            return end();
        }
        // this is a very lazy batch insert, correct way it to implement a
        // range iterator that returns the same value
        auto last_pos = const_cast<iterator>(pos);
        for (size_type ix = 0; ix < count; ++ix) {
            last_pos = insert(last_pos, value);
        }
        return const_cast<iterator>(last_pos-count);
    }

    iterator erase(const_iterator pos) { return erase(pos, pos+1); }
    iterator erase(const_iterator first, const_iterator last) {
        auto count_to_erase = last-first;
        auto distance_to_first = first - begin();
        auto move_to_iter = const_cast<iterator>(first);
        auto last_copy_1 = const_cast<iterator>(last);
        while (last_copy_1!=end()) {
            *(move_to_iter) = microtess::traits::move(*last_copy_1);
            ++last_copy_1; ++move_to_iter;
        }
        while(count_to_erase--!=0) pop_back();
        // pop back might invalidate iterators, so re-invent them
        return begin()+distance_to_first;
    }

    void resize(size_type count) { internal_resize(count, T()); }
    void resize(size_type count, const value_type& value) { internal_resize(count, value); }

    // Iterators
    const_iterator begin() const noexcept {return _data;}
    const_iterator end() const noexcept {return _data + size();}
    iterator begin() noexcept {return _data;}
    iterator end()  noexcept {return _data + size();}

    // etc..
    Alloc get_allocator() const noexcept { return Alloc(_alloc); }
};

template<class T, class Alloc>
bool operator==(const dynamic_array<T,Alloc>& lhs,
                const dynamic_array<T,Alloc>& rhs ) {
    if(!(lhs.size()==rhs.size())) return false;
    using size_type = typename dynamic_array<T,Alloc>::size_type;
    for (size_type ix = 0; ix < lhs.size(); ++ix)
        if(!(lhs[ix]==rhs[ix])) return false;
    return true;
}


///*========================================================================================
// Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
// All Rights Reserved.
// License is a custom open source semi-permissive license with the following guidelines:
// 1. unless otherwise stated, derivative work and usage of this file is permitted and
//    should be credited to the project and the author of this project.
// 2. Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//========================================================================================*/
//#pragma once
//
//namespace dynamic_array_traits {
//
//    template< class T > struct remove_reference      {typedef T type;};
//    template< class T > struct remove_reference<T&>  {typedef T type;};
//    template< class T > struct remove_reference<T&&> {typedef T type;};
//
//    template <class _Tp>
//    inline typename remove_reference<_Tp>::type&&
//    move(_Tp&& __t) noexcept
//    {
//        typedef typename remove_reference<_Tp>::type _Up;
//        return static_cast<_Up&&>(__t);
//    }
//
//    template <class _Tp> inline _Tp&&
//    forward(typename remove_reference<_Tp>::type& __t) noexcept
//    {
//        return static_cast<_Tp&&>(__t);
//    }
//
//    template <class _Tp> inline _Tp&&
//    forward(typename remove_reference<_Tp>::type&& __t) noexcept
//    {
//        return static_cast<_Tp&&>(__t);
//    }
//
//    /**
//     * standard allocator
//     * @tparam T the allocated object type
//     */
//    template<typename T>
//    class std_allocator {
//    public:
//        using value_type = T;
//        using size_t = unsigned long;
//    public:
//        template<class U>
//        explicit std_allocator(const std_allocator<U> & other) noexcept { };
//        explicit std_allocator()=default;
//
//        template <class U, class... Args>
//        void construct(U* p, Args&&... args) {
//            new(p) U(dynamic_array_traits::forward<Args>(args)...);
//        }
//
//        T * allocate(size_t n) { return (T *)operator new(n * sizeof(T)); }
//        void deallocate(T * p, size_t n=0) { operator delete (p); }
//
//        template<class U> struct rebind {
//            typedef std_allocator<U> other;
//        };
//    };
//
//    template<class T1, class T2>
//    bool operator==(const std_allocator<T1>& lhs, const std_allocator<T2>& rhs ) noexcept {
//        return true;
//    }
//}
//
///**
// * Minimal vector like container, does not obey all of the propagate syntax that
// * Allocator Aware Container follows
// * @tparam T the type
// * @tparam Alloc the allocator type
// */
//template<typename T, class Alloc=dynamic_array_traits::std_allocator<T>>
//class dynamic_array {
//    using const_dynamic_array_ref = const dynamic_array<T, Alloc> &;
//public:
//    using value_type = T;
//    using allocator_type = Alloc;
//    using reference = value_type &;
//    using const_reference = const value_type &;
//    using pointer = value_type *;
//    using const_pointer = const value_type *;
//    using iterator = pointer;
//    using const_iterator = const_pointer;
//    using index = unsigned int;
//    using type = T;
//    using uint = unsigned int;
//
//private:
//    using rebind_allocator_type = typename Alloc::template rebind<value_type>::other;
//
//    T *_data = nullptr;
//    rebind_allocator_type _alloc;
//    index _current = 0u;
//    index _cap = 0u;
//
//public:
//    explicit dynamic_array(const Alloc & alloc = Alloc()) noexcept : _alloc{alloc} {
//    }
//
//    dynamic_array(const uint count, const T & value = T(), const Alloc & alloc = Alloc()) :
//            dynamic_array(alloc) {
//        reserve(count);
//        for (int ix = 0; ix < count; ++ix) push_back(value);
//    }
//
//    template<class Iterable>
//    dynamic_array(const Iterable &list, const Alloc & alloc= Alloc()) noexcept :
//            dynamic_array(alloc) {
//        reserve(list.size());
//        for (const auto & item : list) push_back(item);
//    }
//
//    dynamic_array(const dynamic_array & other, const Alloc & alloc) noexcept :
//            dynamic_array(alloc) {
//        reserve(other.size());
//        for (const auto & item : other) push_back(item);
//    }
//
//    dynamic_array(const dynamic_array & other) noexcept :
//            dynamic_array(other, other.get_allocator()) {
//    }
//
//    dynamic_array(dynamic_array && other, const Alloc & alloc) noexcept :
//            dynamic_array(alloc) {
//        reserve(other.size());
//        for (const auto & item : other) push_back(dynamic_array_traits::move(item));
//    }
//
//    dynamic_array(dynamic_array && other) noexcept : dynamic_array{other.get_allocator()} {
//        _data = other._data;
//        _current = other._current;
//        _cap = other._cap;
//        other._data=nullptr;
//        other._cap=0;
//        other._current=0;
//    }
//
//    ~dynamic_array() noexcept { drain(); }
//
//    void reserve(uint new_cap) {
//        bool avoid = new_cap <= capacity();
//        if(avoid) return;
//        T * _new_data = _alloc.allocate(new_cap);
//        // move-construct old objects
//        for (int ix = 0; ix < size(); ++ix)
//            new (_new_data + ix) T(dynamic_array_traits::move(_data[ix]));
//        _cap = new_cap;
//        // no need to destruct because the items were moved
//        if(_data!=nullptr) _alloc.deallocate(_data);
//        _data = _new_data;
//    }
//
//    dynamic_array & operator=(const dynamic_array & other) noexcept {
//        if(this!= &other) {
//            clear();
//            reserve(other.size());
//            for(index ix = 0; ix < other.size(); ix++)
//                push_back(other[ix]);
//        }
//        return (*this);
//    }
//
//    dynamic_array & operator=(dynamic_array && other) noexcept {
//        // two cases:
//        // 1. if the allocators are equal, then move the data completely.
//        // 2. otherwise, move element by element
//        const bool are_equal = _alloc == other.get_allocator();
//        const bool self_assign = this == &other;
//        if(self_assign) return *this;
//
//        if(are_equal) {
//            // clear and destruct current elements
//            clear();
//            // deallocate the current memory
//            _alloc.deallocate(_data, capacity());
//            // move everything from other
//            _data = other._data;
//            _current = other._current;
//            _cap = other._cap;
//            // no need to de-allocate because we stole the memory
//            other._data=nullptr;
//            other._cap=0;
//            other._current=0;
//        } else {
//            if(other.size() >= capacity()) {
//                clear(); // clear and destruct current objects
//                _alloc.deallocate(_data, capacity()); // de allocate the chunk
//                _data = _alloc.allocate(other.size()); // create new chunk
//            }
//            // move other items into current memory
//            for (int ix = 0; ix < other.size(); ++ix)
//                new (_data + ix) T(dynamic_array_traits::move(other[ix]));
//            // no need to destruct other's items
//            _current = other._current;
//            _cap = other._cap;
//            // different allocators, we do not de-allocate the other array.
//        }
//
//        return (*this);
//    }
//
//    T& operator[](index i) noexcept { return _data[i]; }
//    const T& operator[](index i) const noexcept { return _data[i]; }
//    const T& peek() noexcept { return (*this)[_current]; }
//
//    void alloc_(bool up) noexcept {
//        const auto old_size = _current;
//        const auto new_size = up ? (_cap==0?1:_cap*2) : _cap/2;
//        const auto copy_size = old_size<new_size ? old_size : new_size;
//
//        T * _new_data = _alloc.allocate(new_size);
//        // move all previous objects into new location,
//        // therefore we do not need to destruct because we move from same allocator
//        for (index ix = 0; ix < copy_size; ++ix)
//            new (_new_data + ix) T(dynamic_array_traits::move(_data[ix]));
//
//        // de allocate old data
//        _alloc.deallocate(_data, capacity());
//        _data = _new_data;
//        _cap = new_size;
//    }
//
//    void push_back(const T & v) noexcept {
//        if(int(_current)>int(_cap-1)) {
//            // copy the value, edge case if v belongs
//            // to the dynamic array
//            const T vv = v;
//            alloc_(true);
//            new(_data + _current++) T(vv);
//        } else new(_data + _current++) T(v);;
//    }
//
//    void push_back(T && v) noexcept {
//        if(int(_current)>int(_cap-1)) {
//            // copy the value, edge case if v belongs
//            // to the dynamic array
//            const T vv = dynamic_array_traits::move(v);
//            alloc_(true);
//            new(_data + _current++) T(dynamic_array_traits::move(vv));
//        } else new(_data + _current++) T(dynamic_array_traits::move(v));
//    }
//
//    template<class... Args>
//    iterator emplace(const_iterator pos, Args&&... args) {
//        return new (pos) T(dynamic_array_traits::forward<Args>(args)...);
//    }
//
//    template<typename... Args>
//    reference emplace_back(Args&&... args) {
//        if(int(_current)>int(_cap-1)) alloc_(true);
//        new (_data + _current++) T(dynamic_array_traits::forward<Args>(args)...);
//        return back();
//    }
//
//    void push_back(const_dynamic_array_ref other) {
//        const auto count = other.size();
//        for (int ix = 0; ix < count; ++ix) push_back(other[ix]);
//    }
//
//    void pop_back() {
//        if(_current==0) return;
//        _data[_current--].~T();
//        if(_current < (_cap>>1)) alloc_(false);
//    }
//
//    void drain() noexcept {
//        clear();
//        if(_data!=nullptr)
//            _alloc.deallocate(_data, capacity());
//        _data = nullptr;
//        _cap = 0;
//        _current = 0;
//    }
//
//    void clear() noexcept {
//        for (int ix = 0; ix < capacity(); ++ix) _data[ix].~T();
//        _current = 0;
//    }
//    Alloc get_allocator() const noexcept { return Alloc(_alloc); }
//    T* data() noexcept { return _data; }
//    const T* data() const noexcept { return _data; }
//    reference back() noexcept { return _data[_current-1]; }
//    reference front() noexcept { return _data[0]; }
//    const_reference back() const noexcept { return _data[_current-1]; }
//    const_reference front() const noexcept { return _data[0]; }
//    bool empty() noexcept { return _current==0; }
//    index size() const noexcept { return _current; }
//    index capacity() const noexcept { return _cap; }
//    const_iterator begin() const noexcept {return _data;}
//    const_iterator end() const noexcept {return _data + size();}
//    iterator begin() noexcept {return _data;}
//    iterator end()  noexcept {return _data + size();}
//};
//
//template<class T, class Alloc>
//bool operator==(const dynamic_array<T,Alloc>& lhs,
//                const dynamic_array<T,Alloc>& rhs ) {
//    const bool equals_sizes = lhs.size()==rhs.size();
//    if(!equals_sizes) return false;
//    for (int ix = 0; ix < lhs.size(); ++ix) {
//        if(!(lhs[ix]==rhs[ix]))
//            return false;
//    }
//    return true;
//}
