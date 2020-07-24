#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontium/bitmap_font.h>

namespace fontium {
    class AbstractExporter {

    public:
        explicit AbstractExporter() = default;
        virtual ~AbstractExporter()=default;

        const str getErrorString() const { return "m_error_string"; }

        virtual str apply(bitmap_font &data) = 0;
        virtual str fileExtension() = 0;
        virtual str tag() = 0;
    };

}