/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
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
            text_format()=default;

            int leading=0;
            // -1 means use the native size of the font, otherwise
            // it will be scaled.
            int fontSize=-1;
            int letterSpacing=0;
            enum wordWrap wordWrap=wordWrap::normal;
            hAlign horizontalAlign=hAlign::left;
            vAlign verticalAlign=vAlign::top;
            bool kerning=true;
            bool autoScale=false;
        };
    }

}
