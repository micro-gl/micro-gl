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
        if(name=="boxlayouter")
            return new BoxLayouter(config);
        if(name=="boxlayouteroptimized")
            return new BoxLayouterOptimized(config);
        if(name=="gridlayouter")
            return new GridLayouter(config);
        if(name=="gridlinelayouter")
            return new GridLineLayouter(config);
        if(name=="linelayouter")
            return new LineLayouter(config);

        return nullptr;
    }
};