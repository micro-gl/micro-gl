#pragma once

#include <layouters/boxlayouter.h>
#include <layouters/boxlayouteroptimized.h>
#include <layouters/gridlayouter.h>
#include <layouters/gridlinelayouter.h>
#include <layouters/linelayouter.h>

namespace fontium {
    class LayoutFactory {
    public:
        LayoutFactory() = delete;

        static
        AbstractLayout *create(const LayoutConfig *config) {
            str name= config->type;
            if (name == "box")
                return new BoxLayouter(config);
            if (name == "box_optimal")
                return new BoxLayouterOptimized(config);
            if (name == "grid")
                return new GridLayouter(config);
            if (name == "gridline")
                return new GridLineLayouter(config);
            if (name == "line")
                return new LineLayouter(config);

            return new BoxLayouter(config);
        }
    };

}