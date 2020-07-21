#pragma once

#include "../abstractlayouter.h"

class BoxLayouter : public AbstractLayouter
{
public:
    explicit BoxLayouter(const LayoutConfig*  $config) : AbstractLayouter($config) {
    }

    void internal_layout(const QVector<LayoutChar>& chars) override;
};
