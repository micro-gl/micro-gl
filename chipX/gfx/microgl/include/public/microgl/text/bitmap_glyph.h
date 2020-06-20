#pragma once

namespace microgl {
    namespace text {

        struct bitmap_glyph {
            bitmap_glyph() = default;
            bitmap_glyph(int id, int x, int y, int w, int h, int xOffset, int yOffset, int xAdvance) :
                    id{id}, xOffset{xOffset}, yOffset{yOffset}, xAdvance{xAdvance}, x{x}, y{y}, width{w}, height{h} {
            };
            char character;
            /** The unicode ID of the char. */
            int id;
            /** The number of points to move the char in x direction on character arrangement. */
            int xOffset;
            /** The number of points to move the char in y direction on character arrangement. */
            int yOffset;
            /** The number of points the cursor has to be moved to the right for the next char. */
            int xAdvance;
            int x, y;
            int width = -1, height = -1;
//private var _kernings:Dictionary;
        };
    }
}