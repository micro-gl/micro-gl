#pragma once

#include <initializer_list>
#include <microgl/dynamic_array.h>
//#include <microgl/static_array.h>

template<typename T>
class chunker {
    using index = unsigned int;
private:
    using type_ref = T &;
    using type_pointer= T *;
    using const_type_pointer= const T *;
    using chunker_ref = chunker<T> &;
    using const_chunker_ref = const chunker<T> &;
    using dynamic_array_ref = dynamic_array<T> &;
    using const_dynamic_array_ref = const dynamic_array<T> &;

    dynamic_array<T> _data;
    dynamic_array<index> _locations;
//    static_array<T, 512> _data;
//    static_array<index, 512> _locations;

//    C<T, Rest> _data{};
//    C<index, Rest...> _locations{0};
//    C<T, Rest...> aasa;

public:
    struct chunk {
        T* data;
        index size;
    };

    chunker(const_chunker_ref chunker) {
        _data = chunker._data;
        _locations = chunker._locations;
    }

    explicit chunker(unsigned initial_capacity=0) : _data(initial_capacity), _locations() {
        _locations.push_back(0);
    }

    chunker(const std::initializer_list<T> &list) : _data{list}, _locations() {
        _locations.push_back(0);
    }

    void cut_chunk() {
        _locations.push_back(_data.size());
    }

    void push_back(const T & v) {
        _data.push_back(v);
    }

    void push_back(const_dynamic_array_ref container) {
        _data.push_back(container);
    }

    void push_back(const std::initializer_list<T> &list) {
        for(auto it = list.begin(); it != list.end(); ++it)
            _data.push_back(*it);
    }

    void push_back_and_cut(const std::initializer_list<T> &list) {
        push_back(list);
        cut_chunk();
    }

    void push_back_and_cut(dynamic_array_ref container) {
        _data.push_back(container);
        cut_chunk();
    }

    void push_back_and_cut(const_dynamic_array_ref container) {
        _data.push_back(container);
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

