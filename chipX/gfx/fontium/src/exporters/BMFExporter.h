#pragma once

#include <fontium/AbstractExporter.h>

namespace fontium {
// Exporter for Sparrow Framework
// http://www.sparrow-framework.org/
    class BMFExporter : public AbstractExporter {
    public:
        explicit BMFExporter() = default;

        str apply(bitmap_font &data) override;
        str fileExtension() override;
        str tag() override;
    };

}