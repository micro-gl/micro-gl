#pragma once

template<typename E>
class buffer {
public:
    explicit buffer(int size) : buffer<E>(new E[size], size) {
        owner=true;
    }
    buffer(E* $data, int size)  : _data{$data}, _size{size} {}
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
        int size2 = _size;
        for (int ix = 0; ix < size2; ++ix)
            _data[ix] = value;
    }

    E *_data = nullptr;

protected:
    int _size = 0;
    bool owner=false;
};