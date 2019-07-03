#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemberFunctionCanBeStaticInspection"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma clang diagnostic ignored "-Wconstant-conversion"
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma once

#include "Types.h"
#include "PixelFormat.h"

template <typename T>
class CRPT
{
protected:
    T& derived() { return static_cast<T&>(*this); }
    T const& derived() const { return static_cast<T const&>(*this); }
};

template<typename P, typename IMPL>
class PixelCoder : public CRPT<IMPL> {
public:
    PixelCoder(uint8_t bits_red, uint8_t bits_green, uint8_t bits_blue, uint8_t bits_alpha)
                : _bits_per_red{bits_red}, _bits_per_green{bits_green},
                _bits_per_blue{bits_blue}, _bits_per_alpha{bits_alpha} {
        _bpp = _bits_per_red + _bits_per_green + _bits_per_blue + _bits_per_alpha;
    }

    void encode(const color_t & input, P & output) {
        return this->derived().encode(input, output);
    }

    void encode(const color_f_t & input, P & output) {
        return this->derived().encode(input, output);
    }

    void decode(const P & input, color_t & output) {
        return this->derived().decode(input, output);
    }

    void decode(const P & input, color_f_t & output) {
        return this->derived().decode(input, output);
    }

    uint8_t bpp() {
        return this->_bpp;
    }

    uint8_t bits_per_red() {
        return this->_bits_per_red;
    }

    uint8_t bits_per_green() {
        return this->_bits_per_green;
    }

    uint8_t bits_per_blue() {
        return this->_bits_per_blue;
    }

    uint8_t bits_per_alpha() {
        return this->_bits_per_alpha;
    }

    void convert(const color_f_t & input, color_t & output) {
        P pixel;
        encode(input, pixel);
        decode(pixel, output);
    }

    void convert(const color_t & input, color_f_t & output) {
        P pixel;
        encode(input, pixel);
        decode(pixel, output);
    }

    inline PixelFormat format() {
        return this->derived().format();
    };

protected:
    uint8_t _bpp;
    uint8_t _bits_per_red;
    uint8_t _bits_per_green;
    uint8_t _bits_per_blue;
    uint8_t _bits_per_alpha;

};

// array coders
class RGB888_ARRAY : public PixelCoder<vec3<uint8_t>, RGB888_ARRAY> {
public:
    uint8_t MAX = (2 << 8) - 1;

    RGB888_ARRAY() : PixelCoder<vec3<uint8_t>, RGB888_ARRAY>(8, 8, 8, 0) {

    }

    inline void encode(const color_t & input, vec3<uint8_t> & output) {
        output.x=input.r, output.y=input.g, output.z=input.b;
    }

    inline void decode(const vec3<uint8_t> & input, color_t & output) {
        output.r=input.x, output.g=input.y, output.b=input.z, output.a=255;
    };

    inline void encode(const color_f_t & input, vec3<uint8_t> & output) {

        output.x=uint8_t(input.r*MAX), output.y=uint8_t(input.g*MAX), output.z=uint8_t(input.b*MAX);
    }

    inline void decode(const vec3<uint8_t> & input, color_f_t & output) {

        output.r=float(input.x)/MAX, output.g=float(input.y)/MAX, output.b=float(input.z)/MAX;
    };

    inline PixelFormat format() {
        return PixelFormat::RGB888;
    }

};


// 32 bit coders

class RGB888_PACKED_32 : public PixelCoder<uint32_t, RGB888_PACKED_32> {
public:
    uint8_t MAX = (2 << 8) - 1;

    RGB888_PACKED_32() : PixelCoder<uint32_t, RGB888_PACKED_32>(8, 8, 8, 0) {

    }

    inline void encode(const color_t & input, uint32_t & output) {

        output = (input.r << 16) + (input.g << 8) + input.b;
    }

    inline void decode(const uint32_t & input, color_t & output) {
        output.r = (input & 0xFF0000) >> 16;
        output.g = (input & 0x00FF00) >> 8;
        output.b = (input & 0x0000FF);
        output.a = 255;
    };

    inline void encode(const color_f_t & input, uint32_t & output) {
        output = (uint8_t (input.r*MAX) << 16) + (uint8_t (input.g*MAX) << 8) + uint8_t (input.b*MAX);
    }

    inline void decode(const uint32_t & input, color_f_t & output) {
        output.r = ((input & 0xFF0000) >> 16)/float(MAX);
        output.g = ((input & 0x00FF00) >> 8)/float(MAX);
        output.b = (input & 0x0000FF)/float(MAX);
        output.a = 1.0f;
    };

    inline PixelFormat format() {
        return PixelFormat::RGB888;
    }

};

/*

template<typename P>
class PixelCoder {
public:
    virtual inline P encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) = 0;
    virtual inline P encode_from_normalized(float r, float g, float b, float a) = 0;
    virtual inline color_t decode(const P & input) = 0;
    virtual inline color_f_t decode_to_normalized(const P & input) = 0;
    virtual inline PixelFormat format() = 0;
private:
};

// array coders
class RGB888_ARRAY : public PixelCoder<vec3<uint8_t>> {
public:
    uint8_t MAX = (2 << 8) - 1;

    inline vec3<uint8_t> encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return vec3<uint8_t>{r,g,b};
    }

    inline color_t decode(const vec3<uint8_t> & input) override {

        return color_t{input.x, input.y, input.z, 255};
    };

    inline vec3<uint8_t> encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX, g*MAX, b*MAX, MAX);
    }

    inline color_f_t decode_to_normalized(const vec3<uint8_t> & input) override {

        return color_f_t{float(input.x)/MAX, float(input.y)/MAX, float(input.z)/MAX, 1.0f };
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB888;
    }

};

// 32 bit coders

class RGB888_PACKED_32 : public PixelCoder<uint32_t > {
public:
    uint8_t MAX = (2 << 8) - 1;

    inline uint32_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return (r << 16) + (g << 8) + b;
    }

    inline color_t decode(const uint32_t & input) override {
        color_t res;
        res.r = (input & 0xFF0000) >> 16;
        res.g = (input & 0x00FF00) >> 8;
        res.b = (input & 0x0000FF);
        res.a = 255;

        return res;
    };

    inline uint32_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX, g*MAX, b*MAX, MAX);
    }

    inline color_f_t decode_to_normalized(const uint32_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, 1.0f};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB888;
    }

};

class RGBA8888_PACKED_32 : public PixelCoder<uint32_t > {
public:
    uint8_t MAX = (2 << 8) - 1;

    inline uint32_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return (r << 24) + (g << 16) + (b << 8) + a;
    }

    inline color_t decode(const uint32_t & input) override {
        color_t res;

        res.r = (input & 0xFF000000) >> 24;
        res.g = (input & 0x00FF0000) >> 16;
        res.b = (input & 0x0000FF00) >> 8;
        res.a = (input & 0x000000FF);

        return res;
    };

    inline uint32_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*255.0,g*255.0,b*255.0, a*255.0);
    }

    inline color_f_t decode_to_normalized(const uint32_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, 1.0f};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGBA8888;
    }

};

/// 16 bit coders

class RGB565_PACKED_16 : public PixelCoder<uint16_t> {
public:
    uint8_t MAX_R = (2 << 5) - 1;
    uint8_t MAX_G = (2 << 6) - 1;
    uint8_t MAX_B = (2 << 5) - 1;

    inline uint16_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return ((r & 0x1F) << 11) + ((g & 0x3F) << 5) + ((b & 0x1F));
    }

    inline color_t decode(const uint16_t & input) override {
        color_t res;

        res.r = (input & 0xF800) >> 11;         // rrrrr... ........ -> 000rrrrr
        res.g = (input & 0x07E0) >> 5;          // .....ggg ggg..... -> 00gggggg
        res.b = (input & 0x1F);                 // ............bbbbb -> 000bbbbb

        return res;
    };

    inline uint16_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX_R,g*MAX_G,b*MAX_B, 31);
    }

    inline color_f_t decode_to_normalized(const uint16_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX_R, float(res.g)/MAX_G, float(res.b)/MAX_B, 1.0f};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB565;
    }

};

class RGB555_PACKED_16 : public PixelCoder<uint16_t> {
public:
    uint8_t MAX = (2 << 5) - 1;

    inline uint16_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return ((r & 0b00011111) << 10) + ((g & 0b00011111) << 5) + ((b & 0b00011111));
    }

    inline color_t decode(const uint16_t & input) override {
        color_t res;

        res.r = (input & 0b0111110000000000) >> 10;
        res.g = (input & 0b0000001111100000) >> 5;
        res.b = (input & 0b0000000000011111);

        return res;
    };

    inline uint16_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX,g*MAX,b*MAX, MAX);
    }

    inline color_f_t decode_to_normalized(const uint16_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, 1.0f};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB555;
    }

};

class RGBA5551_PACKED_16 : public PixelCoder<uint16_t> {
public:
    uint8_t MAX = (2 << 5) - 1;
    uint8_t MAX_A = 1;

    inline uint16_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return ((r & 0x1F) << 11) + ((g & 0x1F) << 6) + ((b & 0x1F) << 1) + (a & 0x1);
    }

    inline color_t decode(const uint16_t & input) override {
        color_t res;

        res.r = (input & 0b1111100000000000) >> 11;
        res.g = (input & 0b0000011111000000) >> 6;
        res.b = (input & 0b0000000000111110) >> 1;
        res.a = (input & 0b0000000000000001);

        return res;
    };

    inline uint16_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX,g*MAX,b*MAX, a*MAX_A);
    }

    inline color_f_t decode_to_normalized(const uint16_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, float(res.a)};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGBA5551;
    }

};

class RGBA1555_PACKED_16 : public PixelCoder<uint16_t> {
public:
    uint8_t MAX = (2 << 5) - 1;
    uint8_t MAX_A = 1;

    inline uint16_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return ((a & 0x1) << 15) + ((r & 0x1F) << 10) + ((g & 0x1F) << 5) + (b & 0x1F);
    }

    inline color_t decode(const uint16_t & input) override {
        color_t res;

        res.a = (input & 0b1000000000000000) >> 15;
        res.r = (input & 0b0111110000000000) >> 10;
        res.g = (input & 0b0000001111100000) >> 5;
        res.b = (input & 0b0000000000011111);

        return res;
    };

    inline uint16_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX,g*MAX,b*MAX, a*MAX_A);
    }

    inline color_f_t decode_to_normalized(const uint16_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, float(res.a)};
    };

    inline PixelFormat format() override {
        return PixelFormat::ARGB1555;
    }

};

class RGBA4444_PACKED_16 : public PixelCoder<uint16_t> {
public:
    uint8_t MAX = (2 << 4) - 1;

    inline uint16_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return ((r & 0b00001111) << 12) + ((g & 0b00001111) << 8) + ((b & 0b00001111) << 4) + (a & 0b00001111);
    }

    inline color_t decode(const uint16_t & input) override {
        color_t res;

        res.r = (input & 0b1111000000000000) >> 12;
        res.g = (input & 0b0000111100000000) >> 8;
        res.b = (input & 0b0000000011110000) >> 4;
        res.a = (input & 0b0000000000001111);

        return res;
    };

    inline uint16_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX,g*MAX,b*MAX, a*MAX);
    }

    inline color_f_t decode_to_normalized(const uint16_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, float(res.a)/MAX};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGBA4444;
    }

};

// 8 bit coders

class RGB332_PACKED_8 : public PixelCoder<uint8_t > {
public:
    uint8_t MAX_R = (2 << 3) - 1;
    uint8_t MAX_G = (2 << 3) - 1;
    uint8_t MAX_B = (2 << 2) - 1;

    inline uint8_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return ((r & 0b00000111) << 5) + ((g & 0b00000111) << 2) + ((b & 0b00000011));
    }

    inline color_t decode(const uint8_t & input) override {
        color_t res;

        res.r = (input & 0b11100000) >> 5;
        res.g = (input & 0b00011100) >> 2;
        res.b = (input & 0b00000011);

        return res;
    };

    inline uint8_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX_R, g*MAX_G, b*MAX_B, 1);
    }

    inline color_f_t decode_to_normalized(const uint8_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX_R, float(res.g)/MAX_G, float(res.b)/MAX_B, 1.0f};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB332;
    }

};

class RGB8bit : public PixelCoder<uint8_t > {
public:
    uint8_t MAX = (2 << 8) - 1;

    inline uint8_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return r;
    }

    inline color_t decode(const uint8_t & input) override {

        return {input, input, input, MAX};
    };

    inline uint8_t encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX, g*MAX, b*MAX, MAX);
    }

    inline color_f_t decode_to_normalized(const uint8_t & input) override {
        color_t res = decode(input);

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, 1.0f};
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB8;
    }

};
*/
#pragma clang diagnostic pop