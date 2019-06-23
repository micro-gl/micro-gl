#include "../include/microgl/Bitmap.h"

template<typename P>
Bitmap<P>::Bitmap(P* $pixels, int w, int h, PixelCoder<P> * $coder) :
        FrameBuffer<P>($pixels, w * h), _width{w}, _height{h}, _coder{$coder}, _format{$coder->format()} {

}

template<typename P>
Bitmap<P>::Bitmap(int w, int h, PixelCoder<P> * $coder) :
                        Bitmap<P>::Bitmap(new P[w * h], w, h, $coder) {

}

template<typename P>
Bitmap<P>::Bitmap(uint8_t *$pixels, int w, int h, PixelCoder<P> * $coder) :
                        Bitmap<P>::Bitmap(reinterpret_cast<P *>($pixels), w, h, $coder) {

}

template<typename P>
Bitmap<P>::~Bitmap() {
    _width = 0;
    _height = 0;
}

template<typename P>
int Bitmap<P>::width() {
    return _width;
}

template<typename P>
int Bitmap<P>::height() {
    return _height;
}

template<typename P>
PixelFormat &Bitmap<P>::format() {
    return _coder->format();
}

template<typename P>
P Bitmap<P>::pixelAt(int x, int y) {
    return this->_data[y*_width + x];
}

template<typename P>
PixelCoder<P> *Bitmap<P>::coder() {
    return _coder;
}


