#pragma once

#include "../src/exporters/BMFExporter.h"

namespace fontium {
    class ExportFactory {
    public:
        ExportFactory() = delete;

        static
        AbstractExporter *create(const str &name) {
            AbstractExporter * exporter = nullptr;
            if (name == "bmf")
                exporter= new BMFExporter();

            if(exporter==nullptr) {
                throw std::runtime_error{"could not find exporter for :: " + name};
            }

            return exporter;
        }
    };
}