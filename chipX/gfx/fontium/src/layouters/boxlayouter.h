#pragma once

#include "src/AbstractLayout.h"

namespace fontium {
    class BoxLayouter : public AbstractLayout {
    public:
        explicit BoxLayouter(const LayoutConfig *$config) : AbstractLayout($config) {
        }

        void internal_layout(const QVector<LayoutChar> &chars) override;
    };

}