#pragma once

#include "FontConfig.h"
#include "bitmap_font.h"
#include "FontRenderer.h"
#include "LayoutConfig.h"
#include "LayoutFactory.h"
#include "ImageComposer.h"

namespace fontium {

    class Fontium {
    public:
        Fontium()= delete;
        ~Fontium()=delete;

        static
        bitmap_font create(const std::string & name,
                           bytearray & font,
                           FontConfig & font_config,
                           LayoutConfig & layout_config);
    };

}
