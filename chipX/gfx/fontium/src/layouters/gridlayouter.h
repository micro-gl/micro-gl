#pragma once

#include "src/AbstractLayout.h"

namespace fontium {
    class GridLayouter : public AbstractLayout {
    public:
        explicit GridLayouter(const LayoutConfig *$config) : AbstractLayout($config) {
        };
    protected:
        void internal_layout(const QVector<LayoutChar> &chars) override;

        virtual void calculateSize(int maxW, int maxH, size_t count);
    };
}