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

        struct bitmap_glyph {
            bitmap_glyph(int id=0, int x=0, int y=0, int w=-1, int h=-1,
                         int xOffset=0, int yOffset=0, int xAdvance=0) :
                    id{id}, xOffset{xOffset}, yOffset{yOffset}, xAdvance{xAdvance},
                    x{x}, y{y}, width{w}, height{h} {
            };
            /** The unicode ID of the char. */
            int id;
            /** The number of points to move the char in x direction on character arrangement. */
            int xOffset;
            /** The number of points to move the char in y direction on character arrangement. */
            int yOffset;
            /** The number of points the cursor has to be moved to the right for the next char. */
            int xAdvance;
            int x, y;
            int width, height;
        };
    }
}

