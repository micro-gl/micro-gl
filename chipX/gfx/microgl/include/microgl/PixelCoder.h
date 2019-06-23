//
// Created by Tomer Shalev on 2019-06-22.
//

#pragma once
#include "Types.h"
#include "PixelFormat.h"

template<typename P>
class PixelCoder {
public:
    virtual inline P encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) = 0;
    virtual inline P encodeFloat(float r, float g, float b, float a) = 0;
    virtual inline color_f_t decode(const P & input) = 0;
    virtual inline PixelFormat format() = 0;
private:
};

class RGB888_ARRAY : public PixelCoder<vec3<uint8_t>> {
public:
    inline vec3<uint8_t> encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return vec3<uint8_t>{r,g,b};
    }

    inline vec3<uint8_t> encodeFloat(float r, float g, float b, float a) override {

        return encode(r*255.0,g*255.0,b*255.0, 0);
    }

    inline color_f_t decode(const vec3<uint8_t> & input) override {

        return color_f_t{input.x/255.0f, input.y/255.0f, input.z/255.0f, 1.0f };
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB888;
    }

};


/*
class RGB888_PACKED_32 : public PixelCoder<uint32_t > {
public:
    inline uint32_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return encode_RGB888(r,g,b);
    }

    inline uint32_t encodeFloat(float r, float g, float b, float a) override {

        return encode_RGB888(r*255.0,g*255.0,b*255.0);
    }

    inline color_f_t decode(const uint32_t & input) override {
        return decodeFloatRGB(input, PixelFormat::RGB888);
    };

    inline PixelFormat format() override {
        return PixelFormat::RGB888;
    }

};


class RGBA8888_PACKED_32 : public PixelCoder<uint32_t > {
public:
    inline uint32_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return encode_RGBA8888(r,g,b,a);
    }

    inline uint32_t encodeFloat(float r, float g, float b, float a) override {

        return encode_RGBA8888(r*255.0,g*255.0,b*255.0, a*255.0);
    }

    inline color_f_t decode(const uint32_t & input) override {
        return decodeFloatRGB(input, PixelFormat::RGBA8888);
    };

};

class RGB565_PACKED_16 : public PixelCoder<uint16_t> {
public:
    inline uint16_t encode(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override {

        return encode_RGB565(r,g,b);
    }

    inline uint16_t encodeFloat(float r, float g, float b, float a) override {

        return encode(r*31.0,g*63.0,b*31.0, 31);
    }

    inline color_f_t decode(const uint16_t & input) override {
        return decodeFloatRGB(input, PixelFormat::RGB565);
    };

};

*/