#pragma once

#include "src/AbstractLayout.h"

class LineLayouter : public AbstractLayout
{
public:
    explicit LineLayouter(const LayoutConfig*  $config) :
            AbstractLayout($config) {
    };

protected:
    void internal_layout(const QVector<LayoutChar>& input) override ;
};
