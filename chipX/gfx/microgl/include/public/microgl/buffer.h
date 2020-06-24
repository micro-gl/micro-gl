#pragma once

template<typename E>
class buffer {
public:
    buffer(int size)  :
            buffer<E>(new E[size], size) {
        owner=true;
    }

    buffer(E* $data, int size)  : _data{$data}, _size{size} {
        _bpe = sizeof(E);
    }

    ~buffer() {
        if(owner) delete [] _data;
    }
    int size()  const {
        return _size;
    }
    E & readAt(int index) {
        return _data[index];
    }

    void writeAt(const E &value, int index) {
        _data[index] = value;
    }
    const E &operator()(int index) const {
        return _data[index];
    }
    E * data() {
        return _data;
    }
    void fill(const E &value) {
//    memset(_data, value, _size * sizeof(T));
//    memcpy(_data, value, _size * sizeof(T));
//    std::fill(_pixels, &_pixels[_width*_height], value);

        int size2 = _size;
        for (int ix = 0; ix < size2; ++ix) {
            _data[ix] = value;
        }
    }

    E *_data = nullptr;

protected:
    int _size = 0;
    uint8_t _bpe = sizeof(E);
    bool owner=false;
};