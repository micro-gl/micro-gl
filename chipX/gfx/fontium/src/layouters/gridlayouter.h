#pragma once

#include "../abstractlayouter.h"

class GridLayouter : public AbstractLayouter
{
public:
    explicit GridLayouter(const LayoutConfig*  $config) : AbstractLayouter($config) {
    };
protected:
    void internal_layout(const QVector<LayoutChar> &chars) override;

    virtual void calculateSize(int maxW, int maxH, size_t count);
};
