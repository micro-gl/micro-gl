//
// Created by Tomer Shalev on 2019-06-16.
//

#include <PixelFormat.h>

uint8_t countPixelFormatBytes(const PixelFormat &format) {
    switch (format) {

        case RGBA8888:return 4;
        case ARGB8888:return 4;
        case RGBA4444:return 2;
        case ARGB4444:return 2;
        case RGBA5551:return 2;
        case ARGB1555:return 2;
        case RGB888:return 3;
        case RGB565:return 2;
        case RGB332:return 1;
        case RGB555:return 2;
        case RGB8:return 1;
    }
    return 0;
}
