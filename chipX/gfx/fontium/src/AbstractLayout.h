#pragma once

#include "common_types.h"
#include "LayoutResult.h"


namespace fontium {
    struct LayoutConfig;

    class AbstractLayout {
    private:
        const LayoutConfig *m_config;
        LayoutResult m_result;
        int m_compact_w;
        int m_compact_h;

        virtual void processing_hook(QVector<LayoutChar> &chars);

    public:
        explicit AbstractLayout(const LayoutConfig *$config);

        LayoutResult &layout(const vector<LayoutChar> &input); //DoPlace

    protected:
        void resize(int w, int h);

        int width() const;

        int height() const;

        void place(const LayoutChar &);

        virtual void internal_layout(const QVector<LayoutChar> &input) = 0;
    };

}