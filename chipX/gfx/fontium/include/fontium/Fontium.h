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

        class Builder {
        private:
            friend class Fontium;

            FontConfig* _fontConfig=nullptr;
            LayoutConfig* _layoutConfig=nullptr;
            bytearray* _font=nullptr;

        public:
            Builder()= default;

            Builder & fontConfig(FontConfig* fontConfig) {
                _fontConfig=fontConfig;
                return *this;
            }

            Builder & layoutConfig(LayoutConfig* layoutConfig) {
                _layoutConfig=layoutConfig;
                return *this;
            }

            Builder & font(bytearray * data) {
                _font=data;
                return *this;
            }

            Fontium * build() {
                return new Fontium(*this);
            }
        };

        Fontium()= delete;
        ~Fontium()= default;

    protected:
    private:
        FontConfig* _font_config=nullptr;
        LayoutConfig* _layout_config=nullptr;
        bytearray* _font=nullptr;
        AbstractLayout * _layout= nullptr;

        explicit Fontium(const Builder & builder);

    public:
        bitmap_font process(const str& name);
        LayoutConfig & layoutConfig();
        FontConfig & fontConfig();
        AbstractLayout & layoutEngine();
    };

}
