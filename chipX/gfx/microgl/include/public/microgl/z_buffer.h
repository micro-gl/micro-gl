#pragma once

#include <microgl/micro_gl_traits.h>

template<unsigned Bits>
class z_buffer {
private:
    template<bool B, class T, class F>
    using cond=microgl::traits::conditional<B,T,F>;
    static constexpr int log = Bits <= 8 ? 1 : (Bits <= 16 ? 2 : (Bits <= 32 ? 4 : 8));
public:
    using type =typename cond<log == 1, uint8_t, typename cond<log == 2, uint16_t, typename cond<log == 4,
                                                         uint32_t, uint64_t>::type>::type>::type;
private:
    static constexpr type max_value= type((int(1)<<(Bits))-1);
    type * _data= nullptr;
    int _w=0, _h=0, _size=0;
public:
    explicit z_buffer(int w, int h) :
            _data(new type[w * h]), _w{w}, _h{w}, _size{w * h} {
        clear();
    }
    ~z_buffer() { delete [] _data; }
    int width()  const { return _w; }
    int height()  const { return _h; }
    int size()  const { return _size; }
    int sizeOfType()  const { return log; }
    const type &operator[](int index) const { return _data[index]; }
    type &operator[](int index) { return _data[index]; }
    const type &operator()(int x, int y) const{ return _data[y*_w+x]; }
    type &operator()(int x, int y) { return _data[y*_w+x]; }

    int maxValue() const { return max_value; }
    type * data() { return _data; }
    void fill(const int &value) {
        for (int ix = 0; ix < _size; ++ix) _data[ix] = value;
    }
    void clear() { fill(maxValue()); }
};