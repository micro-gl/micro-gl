#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma once

#include "Types.h"
#include "PixelFormat.h"

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

// array coders
class RGBA8888_ARRAY : public PixelCoder<vec4<uint8_t>> {
public:
    uint8_t MAX = (2 << 8) - 1;

    inline vec4<uint8_t> encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return vec4<uint8_t>{r,g,b,a};
    }

    inline color_t decode(const vec4<uint8_t> & input) override {

        return color_t{input.x, input.y, input.z, input.w};
    };

    inline vec4<uint8_t> encode_from_normalized(float r, float g, float b, float a) override {

        return encode(r*MAX, g*MAX, b*MAX, a*MAX);
    }

    inline color_f_t decode_to_normalized(const vec4<uint8_t> & input) override {

        return color_f_t{float(input.x)/MAX, float(input.y)/MAX, float(input.z)/MAX, float(input.w)/MAX };
    };

    inline PixelFormat format() override {
        return PixelFormat::RGBA8888;
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

        return {float(res.r)/MAX, float(res.g)/MAX, float(res.b)/MAX, float(res.a)/MAX};
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

};

#pragma clang diagnostic pop