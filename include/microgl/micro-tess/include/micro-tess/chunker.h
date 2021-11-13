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

namespace chunker_traits {
    template< class T > struct remove_reference      {typedef T type;};
    template< class T > struct remove_reference<T&>  {typedef T type;};
    template< class T > struct remove_reference<T&&> {typedef T type;};

    template <class _Tp> inline typename remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept {
        typedef typename remove_reference<_Tp>::type _Up;
        return static_cast<_Up&&>(__t);
    }
    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type& __t) noexcept {
        return static_cast<_Tp&&>(__t);
    }
    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type&& __t) noexcept {
        return static_cast<_Tp&&>(__t);
    }

    template<typename T=unsigned char>
    class std_rebind_allocator {
    public:
        using value_type = T;
        using size_t = unsigned long;

        template<class U>
        explicit std_rebind_allocator(const std_rebind_allocator<U> & other) noexcept { };
        explicit std_rebind_allocator()=default;

        template <class U, class... Args>
        void construct(U* p, Args&&... args) {
            new(p) U(chunker_traits::forward<Args>(args)...);
        }

        T * allocate(size_t n) { return (T *)operator new(n * sizeof(T)); }
        void deallocate(T * p, size_t n=0) { operator delete (p); }

        template<class U> struct rebind {
            typedef std_rebind_allocator<U> other;
        };
    };

    template<class T1, class T2>
    bool operator==(const std_rebind_allocator<T1>& lhs, const std_rebind_allocator<T2>& rhs ) noexcept {
        return true;
    }
}

template<typename T, template<typename...> class container_template_type,
            class Allocator=chunker_traits::std_rebind_allocator<T>>
class allocator_aware_chunker {
public:
    using index = unsigned int;
    using value_type = T;

    using allocator_type = Allocator;
    using allocator_type_data = typename allocator_type::template rebind<value_type>::other;
    using allocator_type_index = typename allocator_type::template rebind<index>::other;
    using container_data_type = container_template_type<value_type, allocator_type_data>;
    using container_index_type = container_template_type<index, allocator_type_index>;

private:
    using type_pointer = T *;
    using const_type_pointer = const T *;
    using chunker_ref = allocator_aware_chunker &;
    using const_chunker_ref = const allocator_aware_chunker &;

    container_data_type _data;
    container_index_type _locations;

public:
    struct chunk {
    public:
        const T* _data;
        index _size;
    public:
        chunk()=delete;
        chunk(const T* data, index size) : _data(data), _size(size) {}
        const T & operator[](unsigned idx) const { return data()[idx]; }
        const T * data() const { return _data; }
        index size() const { return _size; }
    };
    allocator_aware_chunker(const_chunker_ref val, const allocator_type & allocator=allocator_type()) :
                _data{val._data, allocator_type_data(allocator)},
                _locations{val._locations, allocator_type_index(allocator)} {
    }
    allocator_aware_chunker(allocator_aware_chunker && val) noexcept :
                _data{chunker_traits::move(val._data)}, _locations(chunker_traits::move(val._locations)) {
    }
    explicit allocator_aware_chunker(const allocator_type & allocator=allocator_type())
                : _data(allocator_type_data(allocator)), _locations(allocator_type_index(allocator)) {
        _locations.push_back(0);
    }
    allocator_type get_allocator() {
        return allocator_type(_data.get_allocator());
    }
    chunk back() const { return chunk_for(size()-1); }
    chunk front() { return chunk_for(0); }
    void cut_chunk() { _locations.push_back(_data.size()); }
    void cut_chunk_if_current_not_empty() {
        if(back().size()==0) return;
        _locations.push_back(_data.size());
    }
    void push_back(const T & v) { _data.push_back(v); }
    void push_back(const allocator_aware_chunker & $chunker) {
        cut_chunk_if_current_not_empty();
        for (int ix = 0; ix < $chunker.size(); ++ix) {
            const auto chunk = $chunker[ix];
            for (int jx = 0; jx < chunk.size; ++jx) {
                push_back(chunk.data[jx]);
            }
            cut_chunk_if_current_not_empty();
        }
    }
    template<class Iterable>
    void push_back(const Iterable & list) {
        for (const auto & item : list)
            _data.push_back(item);
    }
    template<class Iterable>
    void push_back_and_cut(const Iterable & container) {
        push_back<Iterable>(container);
        cut_chunk();
    }
    type_pointer data() { return _data.data(); }
    const_type_pointer data() const { return _data.data(); }
    chunker_ref operator=(const_chunker_ref chunker) {
        _data = chunker._data; _locations = chunker._locations;
        return (*this);
    }
    chunker_ref operator=(allocator_aware_chunker && chunker)  noexcept {
        _data = chunker_traits::move(chunker._data);
        _locations = chunker_traits::move(chunker._locations);
        return (*this);
    }
    chunk chunk_for(index i) {
        index idx_start = _locations[i];
        index idx_end = (i+1)<size() ? _locations[i+1] : _data.size();
        index size = idx_end - idx_start;
        type_pointer pointer = &(_data[idx_start]);
        return {pointer, size};
    }
    chunk chunk_for(index i) const {
        index idx_start = _locations[i];
        index idx_end = (i+1)<size() ? _locations[i+1] : _data.size();
        index size = idx_end - idx_start;
        const_type_pointer pointer = &(_data[idx_start]);
        return {pointer, size};
    }
    chunk operator[](index i) { return chunk_for(i); }
    chunk operator[](index i) const { return chunk_for(i); }
    void clear() {
        _locations.clear();
        _data.clear();
        _locations.push_back(0);
    }
    void drain() {
        _locations = container_index_type{};
        _data = container_data_type{};
        _locations.push_back(0);
    }
    bool empty() const { return size()==0; }
    index size() const { return _locations.size(); }
    index unchunked_size() const { return _data.size(); }

    // todo : implement iterator
};

template<typename T, template<typename...> class container_template_type>
class non_allocator_aware_chunker {
public:
    using index = unsigned int;
    using value_type = T;

    using container_data_type = container_template_type<value_type>;
    using container_index_type = container_template_type<index>;

private:
    using type_pointer = T *;
    using const_type_pointer = const T *;
    using chunker_ref = non_allocator_aware_chunker &;
    using const_chunker_ref = const non_allocator_aware_chunker &;

    container_data_type _data;
    container_index_type _locations;

public:
    struct chunk {
    public:
        const T* _data=nullptr;
        index _size;
    public:
        chunk(const T* data, index size) : _data(data), _size(size) {}
        const T & operator[](unsigned idx) const { return data()[idx]; }
        const T * data() const { return _data; }
        index size() const { return _size; }
    };
    non_allocator_aware_chunker(const_chunker_ref val) : _data{val._data}, _locations{val._locations} {
    }
    non_allocator_aware_chunker(non_allocator_aware_chunker && val) noexcept {
        _data = chunker_traits::move(val._data);
        _locations = chunker_traits::move(val._locations);
    }
    explicit non_allocator_aware_chunker() : _data(), _locations() {
        _locations.push_back(0);
    }
    chunk back() const { return chunk_for(size()-1); }
    chunk front() { return chunk_for(0); }
    void cut_chunk() { _locations.push_back(_data.size()); }
    void cut_chunk_if_current_not_empty() {
        if(back().size()==0) return;
        _locations.push_back(_data.size());
    }
    void push_back(const T & v) { _data.push_back(v); }
    void push_back(const non_allocator_aware_chunker & $chunker) {
        cut_chunk_if_current_not_empty();
        for (int ix = 0; ix < $chunker.size(); ++ix) {
            const auto chunk = $chunker[ix];
            for (int jx = 0; jx < chunk.size; ++jx) {
                push_back(chunk.data[jx]);
            }
            cut_chunk_if_current_not_empty();
        }
    }
    template<class Iterable>
    void push_back(const Iterable & list) {
        for (const auto & item : list)
            _data.push_back(item);
    }
    template<class Iterable>
    void push_back_and_cut(const Iterable & container) {
        push_back<Iterable>(container);
        cut_chunk();
    }
    type_pointer data() { return _data.data(); }
    const_type_pointer data() const { return _data.data(); }
    chunker_ref operator=(const_chunker_ref chunker) {
        _data = chunker._data; _locations = chunker._locations;
        return (*this);
    }
    chunker_ref operator=(non_allocator_aware_chunker && chunker)  noexcept {
        _data = chunker_traits::move(chunker._data);
        _locations = chunker_traits::move(chunker._locations);
        return (*this);
    }
    chunk chunk_for(index i) {
        index idx_start = _locations[i];
        index idx_end = (i+1)<size() ? _locations[i+1] : _data.size();
        index size = idx_end - idx_start;
        type_pointer pointer = &(_data[idx_start]);
        return {pointer, size};
    }
    chunk chunk_for(index i) const {
        index idx_start = _locations[i];
        index idx_end = (i+1)<size() ? _locations[i+1] : _data.size();
        index size = idx_end - idx_start;
        const_type_pointer pointer = &(_data[idx_start]);
        return {pointer, size};
    }
    chunk operator[](index i) { return chunk_for(i); }
    chunk operator[](index i) const { return chunk_for(i); }
    void clear() {
        _locations.clear();
        _data.clear();
        _locations.push_back(0);
    }
    void drain() {
        _locations = container_index_type{};
        _data = container_data_type{};
        _locations.push_back(0);
    }
    bool empty() const { return size()==0; }
    index size() const { return _locations.size(); }
    index unchunked_size() const { return _data.size(); }
};

