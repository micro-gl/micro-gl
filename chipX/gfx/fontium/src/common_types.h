#pragma once

#include <iostream>
#include <vector>
#include <map>

namespace fontium {
    using ubyte= unsigned char;
    using uchar= unsigned char;
    using uint= unsigned int;
    using bytearray= std::vector<uchar>;
    using QString = std::string;
    using str = std::string;

    template<typename T>
    using vector = std::vector<T>;

    template<typename T>
    using QVector = std::vector<T>;

    template<typename K, typename V>
    using QMap = std::map<K, V>;

/**
 * unpacked (8|8|8|8) image
 */
    struct Img {
    private:
        ubyte *_data = nullptr;
        uint32_t _w = 0, _h = 0, _channels = 4;

    public:
        Img(uint w, uint h, uint channels = 1) : _w{w}, _h{h}, _channels{channels} {
            if(w*h)
                _data = new ubyte[_channels * _w * _h]{0};
        }

        ~Img() {
            _w = _h = _channels = 0;
            if (_data)
                delete[] _data;
            _data = nullptr;
        }

        ubyte *operator()(uint x, uint y) {
            uint index = (y * _w + x);
            return (*this)[index];
        }

        ubyte *operator[](uint index) {
            return &_data[index * _channels];
        }

        ubyte *data() { return _data; }

        uint width() { return _w; }

        uint height() { return _h; }

        uint channels() { return _channels; }

        void copy(Img &src, uint x, uint y) {
            bool valid = src.channels() == channels();
            if (!valid)
                return;

            uint channels = src.channels();
            uint l = x < 0 ? 0 : x;
            uint t = y < 0 ? 0 : y;
            uint r = x + src.width() >= width() ? width() : x + src.width();
            uint b = y + src.height() >= height() ? height() : y + src.height();
            uint row_start = t - y, row_end = b - y;
            uint col_start = l - x, col_end = r - x;

            for (unsigned row = row_start; row < row_end; ++row) {

                for (unsigned col = col_start; col < col_end; ++col) {
                    uint index_dest = (y + row) * this->width() + (x + col);
                    uint index_src = row * src.width() + col;
                    ubyte *dest_start_pixel = (*this)[index_dest];
                    ubyte *src_start_pixel = src[index_src];
                    // copy 8 bit channels
                    for (uint ix = 0; ix < channels; ++ix) {
                        *(dest_start_pixel++) = (*src_start_pixel++);
                    }
                }

            }
        }

    };
}