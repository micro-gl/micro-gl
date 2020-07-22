#include <sstream>
#include "sparrowexporter.h"

str SparrowExporter::apply(bitmap_font& data) {
    using stream = std::stringstream;
    str M = str{'\"'};
    str NL = str{'\n'};
    str S = " ";
    str T1 = "    ";
    str T2 = T1+T1;
    stream result;
    stream kernings;

    result << "<font face=\"" << data.font_config->family << "\" size=\"" << data.font_config->size << "\" />" << NL;
    result << T1 << "<common lineHeight=\"" << data.metrics.height << "\" />" << NL;
    result << T1 << "<pages>" << NL;
    result << T2 << "<page id=\"0\" file=\"" << data.texture_file << "\"/>" << NL;
    result << T1 << "</pages>";
    result << T1 << "<chars count=\"" << data.chars.size() << "\">" << NL;

    for(const Symbol& c :data.chars) {
        str letter = c.id==32 ? "space" : std::string{char(c.id)};
        result << T2 << "<char" << S;
        result << "x=\"" << c.placeX << M << S;
        result << "y=\"" << c.placeY << M << S;
        result << "width=\"" << c.placeW << M << S;
        result << "height=\"" << c.placeH << M << S;
        result << "xOffset=\"" << c.offsetX << M << S;
        result << "yOffset=\"" << data.metrics.height-c.offsetY << M << S;
        result << "xadvance=\"" << c.advance << M << S;
        result << "page=\"0\" chnl=\"0\"" << S;
        result << "letter=\"" << letter << M << S;
        result << "/>" + NL;
    }
    result << T1 << "</chars>" + NL;

    uint kernings_count=0;
    for(const Symbol& c :data.chars) {
        for (auto const& kerning : c.kerning) {
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
    result << T1 << "</kernings>"<< NL;
    result << "</font>" + NL;

    return result.str();
}

str SparrowExporter::tag() {
    return "sparrow";
}
