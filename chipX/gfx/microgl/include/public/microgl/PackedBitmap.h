#pragma once

#include <microgl/Bitmap.h>

template <unsigned bits, typename CODER>
class PackedBitmap : public Bitmap<uint8_t, CODER> {
    using base=Bitmap<uint8_t , CODER>;
public:
    PackedBitmap(int w, int h) : base{w, h} {};
//    PackedBitmap(P* $pixels, int w, int h) : base{$pixels, w, h} {};
    PackedBitmap(uint8_t* $pixels, int w, int h) : base{$pixels, w, h} {};

    virtual ~PackedBitmap() = default;;

    uint8_t pixelAt(int x, int y) const {
        return 111;
    }

protected:
};