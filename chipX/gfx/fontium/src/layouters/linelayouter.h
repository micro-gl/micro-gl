#pragma once

#include "../abstractlayouter.h"

class LineLayouter : public AbstractLayouter
{
public:
    explicit LineLayouter(const LayoutConfig*  $config) :
                                AbstractLayouter($config) {
    };

protected:
    void internal_layout(const QVector<LayoutChar>& input) override ;
};
