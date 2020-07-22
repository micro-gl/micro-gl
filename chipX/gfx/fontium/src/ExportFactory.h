#pragma once

#include <exporters/sparrowexporter.h>

class ExportFactory {
public:
    ExportFactory()= delete;

    static
    AbstractExporter * create(const str & name) {
        if(name=="sparrowexporter")
            return new SparrowExporter();

        return nullptr;
    }
};