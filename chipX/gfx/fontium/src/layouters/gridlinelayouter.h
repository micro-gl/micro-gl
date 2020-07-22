#pragma once

#include "gridlayouter.h"

namespace fontium {
    class GridLineLayouter : public GridLayouter {
    public:
        explicit GridLineLayouter(const LayoutConfig *$config) : GridLayouter($config) {
        };

    protected:
        void calculateSize(int maxW, int maxH, size_t count) override;
    };
}