#include <sstream>
#include "BMFExporter.h"

namespace fontium {
    str BMFExporter::apply(bitmap_font &data) {
        using stream = std::stringstream;
        str M = str{'\"'};
        str NL = str{'\n'};
        str S = " ";
        str T1 = "    ";
        str T2 = T1 + T1;
        stream result;
        stream kernings;

        result << "<font>" << NL;

        // info
        result << T1 << "<info ";
        result << "face=\"" << data.family << M << S;
        result << "size=\"" <<data.font_config->size << M << S;
        result << "bold=\"" << (data.font_config->bold?1:0) << M << S;
        result << "italic=\"" << (data.font_config->italic?1:0) << M << S;
        result << "stretchH=\"" << (data.font_config->scale_height) << M << S;
        result << "smooth=\"" << (data.font_config->antialiasing ? 1 : 0) << M << S;
        result << "padding=\"" << (data.layout_config->offset_top) << ","
               << data.layout_config->offset_right << "," << data.layout_config->offset_bottom
               << "," << data.layout_config->offset_left << M << S;
        result << "spacing=\"" << (data.font_config->char_spacing) << ","
               << data.font_config->line_spacing << M << S << "/>" << NL;

        // common
        result << T1 << "<common ";
        result << "lineHeight=\"" << data.metrics.height << M << S;
        result << "scaleW=\"" << data.tex_width << M << S;
        result << "scaleH=\"" << data.tex_height << M << S;
        result << "pages=\"1\" packed=\"0\" />" << NL;

        // pages
        result << T1 << "<pages>" << NL;
        result << T2 << "<page id=\"0\" file=\"" << data.image_file_name << "\"/>" << NL;
        result << T1 << "</pages>" << NL;

        // chars
        result << T1 << "<chars count=\"" << data.glyphs.size() << "\">" << NL;
        for (const glyph &c :data.glyphs) {
            result << T2 << "<char" << S;
            result << "id=\"" << c.id << M << S;
            result << "x=\"" << c.placeX << M << S;
            result << "y=\"" << c.placeY << M << S;
            result << "width=\"" << c.placeW << M << S;
            result << "height=\"" << c.placeH << M << S;
            result << "xOffset=\"" << c.offsetX << M << S;
            result << "yOffset=\"" << data.metrics.height - c.offsetY << M << S;
            result << "xadvance=\"" << c.advance << M << S;
            result << "page=\"0\" chnl=\"15\"" << S;
            result << "/>" + NL;
        }
        result << T1 << "</chars>" + NL;

        uint kernings_count = 0;
        for (const glyph &c :data.glyphs) {
            for (auto const &kerning : c.kerning) {
                kernings_count++;
                kernings << T2 << "<kerning" << S;
                kernings << "first=\"" << c.id << M << S;
                result << "second=\"" << kerning.first << M << S;
                result << "amount=\"" << kerning.second << M << S;
                result << "/>" + NL;
            }
        }

        result << T1 << "<kernings count=\"" << kernings_count << "\">" << NL;
        result << kernings.str();
        result << T1 << "</kernings>" << NL;
        result << "</font>" + NL;

        return result.str();
    }

    str BMFExporter::tag() {
        return "bmf";
    }

    str BMFExporter::fileExtension() {
        return "xml";
    }

}