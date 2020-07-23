#pragma once

#include <FontConfig.h>
#include <bitmap_font.h>
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
        ~Fontium()= default;

    protected:
    private:
        FontConfig*     _font_config;
        LayoutConfig*   _layout_config;
        bytearray* _font=nullptr;
        AbstractLayout * _layout= nullptr;

        explicit Fontium(const Builder & builder) {
            _font_config=builder._fontConfig;
            _layout_config=builder._layoutConfig;
            _font=builder._font;
            _layout = LayoutFactory::create(_layout_config);
        }

    public:
        bitmap_font process(str name) {

            FontRenderer fontRenderer{_font, _font_config};
            FontRendererResult fontRendererResult = fontRenderer.render(1.0f);
            // collect base chars to prepare for layout
            std::vector<LayoutChar> ll{};
            ll.reserve(fontRendererResult.chars.size());
            for(const auto & entry: fontRendererResult.chars) {
                const auto & r = entry.second;
                ll.emplace_back(r.symbol, r.offsetX, -r.offsetY, r.w, r.h);
            }
            auto layout_result = layoutEngine().layout(ll);
            Img * img= ImageComposer::compose(layout_result,
                                              layoutConfig(), fontRendererResult);
            auto bm_font = bitmap_font::from(img, name, layout_result,
                                             fontRendererResult,
                                             fontConfig(),
                                             layoutConfig());
            bm_font.name= name;
            bm_font.family= fontRendererResult.family;
            bm_font.style= fontRendererResult.style;

            fontRendererResult.dispose();

            return bm_font;
        }

        LayoutConfig & layoutConfig() {
            return *_layout_config;
        }

        FontConfig & fontConfig() {
            return *_font_config;
        }

        AbstractLayout & layoutEngine() {
            return *_layout;
        }

    };

}
