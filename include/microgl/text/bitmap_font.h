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

#include "bitmap_glyph.h"
#include "text_format.h"

namespace microgl {
    namespace text {
        struct char_location {
            int x=0, y=0;
            bitmap_glyph * character= nullptr;
        };
        struct text_layout_result {
            int scale=1; int precision=0;
            int end_index=0;
            char_location * locations=nullptr;
        };

        /**
         * bitmap font
         * @tparam bitmap_type the type of bitmap
         * @tparam MAX_CHARS max number of glyphs
         */
        template<typename bitmap_type, unsigned MAX_CHARS=100>
        class bitmap_font {
            static const int CHAR_MISSING = 0;
            static const int CHAR_TAB = 9;
            static const int CHAR_NEWLINE = 10;
            static const int CHAR_CARRIAGE_RETURN = 13;
            static const int CHAR_SPACE = 32;
            bitmap_glyph char_missing =
                    bitmap_glyph{CHAR_MISSING, 0, 0, 0, 0, 0, 0, 0};
            int count_internal = 0;
        public:
            /** The name of the font as it was parsed from_sampler the font file. */
            char name[20]={};
            /** The native size of the font. */
            int nativeSize=0;
            /** The height of one line in points. */
            int lineHeight=0;
            /** The baseline of the font. This property does not affect text rendering;
              *  it's just an information that may be useful for exact text placement. */
            int baseline=0;
            /** An offset that moves any generated text along the x-axis (in points).
              *  Useful to make up for incorrect font data. @default 0. */
            int offsetX = 0;
            /** An offset that moves any generated text along the y-axis (in points).
              *  Useful to make up for incorrect font data. @default 0. */
            int offsetY = 0;
            /** The width of a "gutter" around the composed text area, in points.
              *  This can be used to bring the output more in line with standard TrueType rendering:
              *  Flash always draws them with 2 pixels of padding. @default 0.0 */
            int padding = 0;
            int glyphs_count = 0;
            int width=0, height=0;
            bitmap_type * bitmap = nullptr;
            bitmap_glyph gylphs[MAX_CHARS];

        public:
            bitmap_font() { addChar(CHAR_MISSING, 0,0,0,0,0,0,0); }

            void addChar(int id, int x, int y, int w, int h, int xOffset, int yOffset, int xAdvance) {
                gylphs[count_internal++] = bitmap_glyph{id, x, y, w, h, xOffset, yOffset, xAdvance};
            }

            bitmap_glyph *charByID(int id) {
                for (int ix = 0; ix < count_internal; ++ix)
                    if (gylphs[ix].id == id) return &gylphs[ix];
                return nullptr;
            }

            text_layout_result layout_text(
                    const char * text, int numChars,
                    int box_width, int box_height,
                    text_format format,
                    char_location * locations_buffer)
            {
                int PP=4;
                text_layout_result result;
                result.locations=locations_buffer;
                result.precision=PP;
                if (text == nullptr || numChars == 0) return result;

                int fontSize = format.fontSize<0 ? nativeSize : format.fontSize;
                bool autoScale = false;
                bool finished = false;
                int containerWidth, containerHeight;
                int scale=0;
                int size=nativeSize<<PP;
                int currentX=0, currentY=0;
                int start_loc_index=-1, loc_idx=0;
                while (!finished)
                {
                    scale = (fontSize<<PP) / nativeSize;
                    containerWidth  = (((box_width - 2 * padding) << PP) << PP) / scale;
                    containerHeight = (((box_height - 2 * padding) << PP) << PP) / scale;
                    if (size <= containerHeight) // && autoScale
                    {
                        int lastWhiteSpace=-1, lastCharID=-1;
                        for (int ix=0; ix<numChars; ++ix)
                        {
                            bool lineFull = false;
                            int charID = text[ix];
                            auto *bitmap_char = charByID(charID);
                            if (charID == CHAR_NEWLINE || charID == CHAR_CARRIAGE_RETURN)
                                lineFull = true;
                            else {
                                if (bitmap_char == nullptr) {
                                    charID = CHAR_MISSING;
                                    bitmap_char = &char_missing;
                                }

                                if (charID==CHAR_SPACE || charID==CHAR_TAB) lastWhiteSpace = ix;
//                                if (kerning)
//                                    currentX += char.getKerning(lastCharID);

                                if(start_loc_index==-1) start_loc_index=loc_idx;
                                char_location & loc = locations_buffer[loc_idx++];
                                loc.character=bitmap_char;
                                loc.x = currentX + (bitmap_char->xOffset<<PP);
                                loc.y = currentY + (bitmap_char->yOffset<<PP);
                                currentX += (bitmap_char->xAdvance + format.letterSpacing)<<PP;
                                lastCharID = charID;
                                bool does_overflow=loc.x + ((bitmap_char->width)<<PP) > containerWidth;
                                if (does_overflow) {
                                    switch (format.wordWrap) {
                                        case wordWrap::break_word:
                                        {
                                            // when autoscaling, we must not split a word in half -> restart
                                            if (autoScale && lastWhiteSpace == -1) break;
                                            // remove characters and add them again to next line
                                            int numCharsToRemove = lastWhiteSpace == -1 ? 1 : ix - lastWhiteSpace;
                                            if ((loc_idx-=numCharsToRemove)==0) break;
                                            ix -= numCharsToRemove;
                                            break;
                                        }
                                        case wordWrap::normal:
                                        {
                                            if (autoScale) break;
                                            loc_idx-=1;
                                            // continue with next line, if there is one
                                            while (ix++<numChars-1 && text[ix]!=CHAR_NEWLINE && text[ix]!=CHAR_SPACE
                                                                                                && text[ix]!=CHAR_TAB);
                                            break;
                                        }
                                    }
                                    lineFull = true;
                                }
                            }

                            if (ix==numChars-1) {
                                finished = true;
                                lineFull=true; //tomer
                            }

                            if (lineFull) {
                                int end_loc_index=loc_idx-1;
                                if (lastWhiteSpace==ix) end_loc_index-=1;
                                if (format.horizontalAlign!=hAlign::left) {
                                    int layoutOffset=0;
                                    const auto last_char_loc=locations_buffer[end_loc_index];
                                    layoutOffset = last_char_loc.x- ((last_char_loc.character->xOffset-
                                            last_char_loc.character->xAdvance)<<PP);
                                    layoutOffset= containerWidth-layoutOffset;
                                    if (format.horizontalAlign==hAlign::center) layoutOffset/=2;
                                    for (int jj=start_loc_index; jj<=end_loc_index; ++jj)
                                        locations_buffer[jj].x+=layoutOffset;
                                }
                                currentX = 0; currentY += (lineHeight + format.leading)<<PP;
                                start_loc_index=lastCharID=lastWhiteSpace = -1;
                                if ((currentY + size+ ((lineHeight + format.leading)<<PP)) > containerHeight)
                                    break;
                            }
                        } // for each char
                    } // if (_lineHeight <= containerHeight)

                    if (autoScale && !finished && fontSize > 3) fontSize -= (1<<PP);
                    else finished = true;
                } // while (!finished)

                int layout_v_offset=0;
                if (format.verticalAlign!=vAlign::top) {
                    int bottom=currentY;// + (lineHeight<<PP);
                    layout_v_offset = containerHeight - bottom; // bottom
                    if (format.verticalAlign==vAlign::center) layout_v_offset/=2;
                }
                for (int jj=0; jj<loc_idx; ++jj) {
                    auto & char_final_loc=locations_buffer[jj];
                    char_final_loc.x = (scale * (char_final_loc.x + (offsetX<<PP)))>>PP;
                    char_final_loc.y = (scale * (char_final_loc.y + layout_v_offset + (offsetY<<PP)))>>PP;
                    char_final_loc.x += (padding<<PP); char_final_loc.y += (padding<<PP);
                }
                result.scale=scale;
                result.end_index=loc_idx;
                return result;
            }
        };
    }
}