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

#include "./stdint.h"
#include "traits.h"

namespace microgl {
    template<unsigned Bits, class Allocator=microgl::traits::std_rebind_allocator<>>
    class z_buffer {
    private:
        template<bool B, class T, class F>
        using cond = microgl::traits::conditional<B, T, F>;
        static constexpr int log = Bits <= 8 ? 1 : (Bits <= 16 ? 2 : 4);
    public:
        using value_type = typename cond<log == 1, microgl::ints::uint8_t, typename cond<log == 2, microgl::ints::uint16_t, microgl::ints::uint32_t>::type>::type;
        using allocator_type = typename Allocator::template rebind<value_type>::other;
        static constexpr int bits = Bits;
        static constexpr value_type max_value = ~value_type(0);
    private:
        allocator_type _allocator;
        value_type *_data = nullptr;
        int _w = 0, _h = 0, _size = 0;
    public:
        explicit z_buffer(int w, int h, const Allocator &allocator = Allocator()) :
                _allocator(allocator), _data(_allocator.allocate(w * h)), _w{w}, _h{w}, _size{w * h} {
            clear();
        }
        ~z_buffer() { _allocator.deallocate(_data); }
        int width() const { return _w; }
        int height() const { return _h; }
        int size() const { return _size; }
        int sizeOfType() const { return log; }
        const value_type &operator[](int index) const { return _data[index]; }
        value_type &operator[](int index) { return _data[index]; }
        const value_type &operator()(int x, int y) const { return _data[y * _w + x]; }
        value_type &operator()(int x, int y) { return _data[y * _w + x]; }
        constexpr int maxValue() const { return max_value; }
        value_type *data() { return _data; }
        void fill(const int &value) { for (int ix = 0; ix < _size; ++ix) _data[ix] = value; }
        void clear() { fill(maxValue()); }
    };
}