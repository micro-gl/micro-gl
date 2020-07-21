#pragma once

#include "boxlayouter.h"

class BoxLayouterOptimized : public BoxLayouter
{
public:
    explicit BoxLayouterOptimized(const LayoutConfig*  $config) : BoxLayouter($config) {
    }
private:
    static bool SortCharsByHeight(const LayoutChar &a, const LayoutChar &b);
    void processing_hook(QVector<LayoutChar> &chars) override;
};
