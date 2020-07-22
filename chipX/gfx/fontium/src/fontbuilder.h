//#pragma once
//
//#include "fontconfig.h"
//#include "fontrenderer.h"
//#include "layoutconfig.h"
//#include "layouterfactory.h"
//#include "exporterfactory.h"
//
//class FontBuilder {
//public:
//
//    class Builder {
//    private:
//        friend class FontBuilder;
//        FontConfig* _fontConfig=nullptr;
//        LayoutConfig* _layoutConfig=nullptr;
//        bytearray* _font=nullptr;
//    public:
//        Builder() {
//
//        }
//
//        Builder & fontConfig(FontConfig* fontConfig) {
//            _fontConfig=fontConfig;
//            return *this;
//        }
//
//        Builder & layoutConfig(LayoutConfig* layoutConfig) {
//            _layoutConfig=layoutConfig;
//            return *this;
//        }
//
//        Builder & font(bytearray * data) {
//            _font=data;
//            return *this;
//        }
//
//        FontBuilder * build() {
//            return new FontBuilder(*this);
//        }
//    };
//
//    FontBuilder()= delete;
//    ~FontBuilder();
//
//protected:
//private:
//    FontConfig*     _font_config;
//    LayoutConfig*   _layout_config;
//    bytearray* _font=nullptr;
//    FontBuilder(const Builder & builder) {
//        _font_config=builder._fontConfig;
//        _layout_config=builder._layout_config;
//        _font=builder._font;
//    }
//
//
//
//
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
//
//};
