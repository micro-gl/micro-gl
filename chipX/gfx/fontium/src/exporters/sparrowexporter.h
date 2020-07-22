#pragma once

#include "src/AbstractExporter.h"

namespace fontium {
// Exporter for Sparrow Framework
// http://www.sparrow-framework.org/
    class SparrowExporter : public AbstractExporter {
    public:
        explicit SparrowExporter() = default;

        str apply(bitmap_font &data) override;

        str tag() override;
    };

}