#pragma once

#include <string>
#include <map>
#include <cstdlib>
#include <utility>
#include "bundle.h"
#include "utils.h"
#include <fontium/FontConfig.h>
#include <fontium/LayoutConfig.h>

namespace fontium {

    class options {
    public:
        FontConfig fontConfig;
        LayoutConfig layoutConfig;
        str output_export_type;
        str output_export_name;
        str input_font_path="";

    private:
        bundle _bundle;
    public:
        options()= default;
        explicit options(bundle & bundle) : _bundle{std::move(bundle)} {
            // parse the bundle into concrete typed variables
            // input
            input_font_path=_bundle.getValueAsString("VOID_KEY", "");
            // output
            output_export_name=_bundle.getValueAsString("output.name",
                    filename(input_font_path));
            output_export_type=_bundle.getValueAsString("output.export", "bmf");
            // font config
            fontConfig.size = _bundle.getValueAsInteger("font.size", 14);
            fontConfig.dpi = _bundle.getValueAsInteger("font.dpi", 72);
            fontConfig.bold = _bundle.getValueAsInteger("font.bold", 0);
            fontConfig.face_index = _bundle.getValueAsInteger("font.face_index", 0);
            fontConfig.italic = _bundle.getValueAsInteger("font.italic", 0);
            fontConfig.line_spacing = _bundle.getValueAsInteger("font.line_spacing", 0);
            fontConfig.char_spacing = _bundle.getValueAsInteger("font.char_spacing", 0);
            fontConfig.scale_width = _bundle.getValueAsFloat("font.scale_width", 100.0f);
            fontConfig.scale_height = _bundle.getValueAsFloat("font.scale_height", 100.0f);
            fontConfig.characters = _bundle.getValueAsString("font.characters",
                    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
            fontConfig.antialiasing = FontConfig::stringToAAEnum(
                    _bundle.getValueAsString("font.antialiasing", "Normal"));
            fontConfig.hinting = FontConfig::stringToHintingEnum(
                    _bundle.getValueAsString("font.hinting", "Default"));
            // layout config
            layoutConfig.layout_type = LayoutConfig::stringToLayoutTypeEnum(
                    _bundle.getValueAsString("layout.type", "box"));
            layoutConfig.size_increment = _bundle.getValueAsInteger("layout.size_increment", 0);
            layoutConfig.offset_left = _bundle.getValueAsInteger("layout.offset_left", 0);
            layoutConfig.offset_top = _bundle.getValueAsInteger("layout.offset_top", 0);
            layoutConfig.offset_right = _bundle.getValueAsInteger("layout.offset_right", 0);
            layoutConfig.offset_bottom = _bundle.getValueAsInteger("layout.offset_bottom", 0);
            layoutConfig.one_pixel_offset = _bundle.getValueAsBoolean("layout.one_pixel_offset", true);
            layoutConfig.pot_image = _bundle.getValueAsBoolean("layout.pot_image", false);

        }

        bundle & extraOptions() { return _bundle; }
    };
}
