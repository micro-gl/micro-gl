#pragma once

#include <layouters/boxlayouter.h>
#include <layouters/boxlayouteroptimized.h>
#include <layouters/gridlayouter.h>
#include <layouters/gridlinelayouter.h>
#include <layouters/linelayouter.h>

class LayoutFactory {
public:
    LayoutFactory()= delete;

    static
    AbstractLayout * create(const str & name, const LayoutConfig * config) {
        if(name=="box")
            return new BoxLayouter(config);
        if(name=="box_optimal")
            return new BoxLayouterOptimized(config);
        if(name=="grid")
            return new GridLayouter(config);
        if(name=="gridline")
            return new GridLineLayouter(config);
        if(name=="line")
            return new LineLayouter(config);

        return new BoxLayouter(config);
    }
};