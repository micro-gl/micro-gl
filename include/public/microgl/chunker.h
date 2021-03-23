#pragma once

#include <microgl/dynamic_array.h>
#include <microgl/micro_gl_traits.h>

template<typename T>
class chunker {
    using index = unsigned int;
    using type = T;
    using container_data_type = dynamic_array<type>;
    using container_index_type = dynamic_array<index>;

private:
    using type_ref = T &;
    using type_pointer= T *;
    using const_type_pointer= const T *;
    using chunker_ref = chunker<T> &;
    using const_chunker_ref = const chunker<T> &;

    container_data_type _data;
    container_index_type _locations;

public:
    struct chunk {
        const T* data;
        index size;
//        index offset;
    };

    chunker(const_chunker_ref val) {
        _data = val._data;
        _locations = val._locations;
    }

    chunker(chunker<T> && val) noexcept {
        _data = traits::move(val._data);
        _locations = traits::move(val._locations);
    }
    explicit chunker(unsigned initial_capacity=0) : _data(initial_capacity), _locations() {
        _locations.push_back(0);
    }

    chunk back() const {
        return chunk_for(size()-1);
    }

    chunk front() {
        return chunk_for(0);
    }

    void cut_chunk() {
        _locations.push_back(_data.size());
    }

    void cut_chunk_if_current_not_empty() {
        if(back().size==0) return;
        _locations.push_back(_data.size());
    }

    void push_back(const T & v) {
        _data.push_back(v);
    }

    void push_back(const chunker & $chunker) {
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

    type_pointer raw_data() {
        return _data.data();
    }

    const_type_pointer raw_data() const {
        return _data.data();
    }

    chunker_ref operator=(const_chunker_ref chunker) {
        _data = chunker._data;
        _locations = chunker._locations;
        return (*this);
    }

    chunker_ref operator=(chunker<T> && chunker)  noexcept {
        _data = traits::move(chunker._data);
        _locations = traits::move(chunker._locations);
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

    chunk operator[](index i) {
        return chunk_for(i);
    }

    chunk operator[](index i) const {
        return chunk_for(i);
    }

    void clear() {
        _locations.clear();
        _data.clear();
        _locations.push_back(0);
    }

    void drain() {
        _locations.drain();
        _data.drain();
        _locations.push_back(0);
    }

    bool empty() const {
        return size()==0;
    }

    index size() const {
        return _locations.size();
    }

    index unchunked_size() const {
        return _data.size();
    }

};

