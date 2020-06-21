#pragma once

namespace microgl {
    namespace text {
        enum class hAlign {
            left, center, right
        };
        enum class vAlign {
            top, center, bottom
        };
        enum wordWrap {
            normal, break_word
        };

        struct text_format {
            int leading=0;
            int fontSize=-1;
            int letterSpacing=0;
            wordWrap wordWrap=wordWrap::normal;
            hAlign horizontalAlign=hAlign::left;
            vAlign verticalAlign=vAlign::top;
            bool kerning=true;
            bool autoScale=false;
        };
    }

}
