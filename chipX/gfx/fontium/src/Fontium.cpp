#include <fontium/Fontium.h>

namespace fontium {
    Fontium::Fontium(const Fontium::Builder &builder) {
        _font_config=builder._fontConfig;
        _layout_config=builder._layoutConfig;
        _font=builder._font;
        _layout = LayoutFactory::create(_layout_config);
    }

    bitmap_font Fontium::process(const str &name) {

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
                                          layoutConfig(),
                                          fontRendererResult);
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

    LayoutConfig &Fontium::layoutConfig() {
        return *_layout_config;
    }

    FontConfig &Fontium::fontConfig() {
        return *_font_config;
    }

    AbstractLayout &Fontium::layoutEngine() {
        return *_layout;
    }

}