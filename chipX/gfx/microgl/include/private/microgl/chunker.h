#pragma once

#include <microgl/dynamic_array.h>

template<typename T>
class chunker {
    using index = unsigned int;
private:
    using type_ref = T &;
    using type_pointer= T *;
    using chunker_ref = chunker<T> &;
    using const_chunker_ref = chunker<T> &;
    using dyn_array_ref = dynamic_array<T> &;
    using array_container_ref = array_container<T> &;
    using const_array_container_ref = const array_container<T> &;

    dynamic_array<T> _data;
    dynamic_array<index> _locations;
//    C<T, Rest> _data{};
//    C<index, Rest...> _locations{0};
//    C<T, Rest...> aasa;

public:
    struct chunk {
        T* data;
        index size;
    };

//    chunker(const_chunker_ref chunker) {
//        _data = chunker._data;
//        _locations = chunker._locations;
//    }

    chunker(const chunker<T> &chunker) {
        _data = chunker._data;
        _locations = chunker._locations;
    }

    explicit chunker(unsigned initial_capacity=0) : _data{initial_capacity}, _locations(initial_capacity) {
        _locations.push_back(0);
    }

    chunker(const std::initializer_list<T> &list) : _data{list}, _locations(1) {
        _locations.push_back(0);
    }

    chunker(const_array_container_ref container) : _data(container), _locations(1) {
        _locations.push_back(0);
    }

    ~chunker() {
    }

    void cut_chunk() {
        _locations.push_back(_data.size());
    }

    void push_back(const T & v) {
        _data.push_back(v);
    }

    void push_back(const_array_container_ref container) {
        _data.push_back(container);
    }

    void push_back(const std::initializer_list<T> &list) {
        _data.push_back(dynamic_array<T> {list});
    }

    type_pointer raw_data() {
        return _data.data();
    }

    type_pointer raw_data() const {
        return _data.data();
    }

    chunker_ref operator=(const_chunker_ref chunker) {
        _data = chunker._data;
        _locations = chunker._locations;

        return (*this);
    }

    chunk chunk_for(index i) {
        index idx_start = _locations[i];
        index size = _locations[i+1] - idx_start;
        type_pointer pointer = &(_data[idx_start]);

        return {pointer, size};
    }

    chunk operator[](index i) {
        return chunk_for(i);
    }

//    chunk operator[](index i) const {
//        return chunk_for(i);
//    }

    void clear() {
    }

    bool empty() const {
        return size()==0;
    }

    index size() const {
        return _locations.size() - 1;
    }

};
