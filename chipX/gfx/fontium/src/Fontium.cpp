#include <fontium/Fontium.h>

namespace fontium {

    bitmap_font Fontium::create(const std::string & name,
                                bytearray &font,
                                FontConfig &font_config,
                                LayoutConfig &layout_config) {

        FontRenderer fontRenderer{font, font_config.face_index};
        FontRendererResult fontRendererResult = fontRenderer.render(font_config, 1.0f);
        // collect base chars to prepare for layout
        std::vector<LayoutChar> ll{};
        ll.reserve(fontRendererResult.chars.size());
        for(const auto & entry: fontRendererResult.chars) {
            const auto & r = entry.second;
            ll.emplace_back(r.symbol, r.offsetX, -r.offsetY, r.w, r.h);
        }
        auto * layout= LayoutFactory::create(layout_config);
        auto layout_result = layout->layout(ll);
        Img * img= ImageComposer::compose(layout_result,
                                          layout_config,
                                          fontRendererResult);
        auto bm_font = bitmap_font::from(img, name, layout_result,
                                         fontRendererResult,
                                         font_config,
                                         layout_config);
        bm_font.name= name;
        bm_font.family= fontRendererResult.family;
        bm_font.style= fontRendererResult.style;

        fontRendererResult.dispose();

        delete layout;
        return bm_font;
    }

}