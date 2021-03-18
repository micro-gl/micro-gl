#pragma once

template<typename E>
class buffer {
public:
    using element_type = E;

private:

    void copy_from(const buffer & val) {
        destroyIfPossibleAndReset();
        _data = new element_type[val.size()];
        _size = val.size();
        owner = true;
        for (int ix = 0; ix < _size; ++ix) {
            _data[ix]=val._data[ix];
        }
    }

    void move_from(buffer & val) {
        destroyIfPossibleAndReset();
        _data = val.data();
        _size = val.size();
        owner = val.owner;
        val.owner = false;
        val.destroyIfPossibleAndReset();
    }

public:

    buffer()=default;
    explicit buffer(int size) : buffer<element_type>(new element_type[size], size, true) {}
    buffer(element_type* $data, int size, bool owner=false)  : _data{$data}, _size{size}, owner{owner} {}
    buffer(const buffer & val) { copy_from(val); }
    buffer(buffer && val) noexcept { move_from(val); }
    buffer & operator=(const buffer & val) {
        if(&val==this) return *this;
        copy_from(val);
        return *this;
    }
    buffer & operator=(buffer && val) noexcept { move_from(val); return *this; }

    void destroyIfPossibleAndReset() {
        if(owner)
            delete [] _data;
        _data= nullptr;
        _size=0;
    };

    ~buffer() {
        destroyIfPossibleAndReset();
    }
    int size()  const {
        return _size;
    }
    element_type & readAt(int index) {
        return _data[index];
    }

    void writeAt(const element_type &value, int index) {
        _data[index] = value;
    }
    const element_type &operator()(int index) const {
        return _data[index];
    }
    element_type * data() {
        return _data;
    }
    void fill(const element_type &value) {
        int size2 = _size;
        for (int ix = 0; ix < size2; ++ix)
            _data[ix] = value;
    }

    element_type *_data = nullptr;
    bool owner=false;

protected:
    int _size = 0;
};