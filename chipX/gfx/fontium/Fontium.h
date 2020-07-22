#pragma once

#include <FontConfig.h>
#include <bitmap_font.h>
#include "FontRenderer.h"
#include "LayoutConfig.h"
#include "layouterfactory.h"
#include "exporterfactory.h"

class Fontium {
public:

    class Builder {
    private:
        friend class Fontium;
        FontConfig* _fontConfig=nullptr;
        LayoutConfig* _layoutConfig=nullptr;
        bytearray* _font=nullptr;
    public:
        Builder() {

        }

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
    ~Fontium();

protected:
private:
    FontConfig*     _font_config;
    LayoutConfig*   _layout_config;
    bytearray* _font=nullptr;
    bitmap_font _result;
    Fontium(const Builder & builder) {
        _font_config=builder._fontConfig;
        _layout_config=builder._layoutConfig;
        _font=builder._font;
    }

    bitmap_font & process(str name) {
        FontRenderer fontRenderer{_font, _font_config};
        FontRendererResult fontRendererResult = fontRenderer.render(1.0f);
        // collect base chars to prepare for layout
        std::vector<LayoutChar> ll{fontRendererResult.chars.size()};
        for(const auto & entry: fontRendererResult.chars) {
            const auto & r = entry.second;
            ll.emplace_back(r.symbol, r.offsetX, r.offsetY, r.w, r.h);
        }
    }




//    FontRenderer*   m_font_renderer;
////    LayoutData*     m_layout_data;
//    AbstractLayout* m_layouter;
//    LayouterFactory*    m_layouter_factory;
//    OutputConfig*   m_output_config;
//    ExporterFactory* m_exporter_factory;
//    ImageWriterFactory* m_image_writer_factory;
//    AbstractImageWriter* m_image_writer;
//
//    void doExport(bool x2);

};
