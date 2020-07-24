#pragma once

#include "common_types.h"
#include "LayoutResult.h"


namespace fontium {
    struct LayoutConfig;

    class AbstractLayout {
    private:
        const LayoutConfig *m_config=nullptr;
        LayoutResult m_result{};
        int m_compact_w=0;
        int m_compact_h=0;

        virtual void processing_hook(QVector<LayoutChar> &chars);

    public:
        explicit AbstractLayout(const LayoutConfig *$config);
        virtual ~AbstractLayout()= default;

        LayoutResult &layout(const vector<LayoutChar> &input); //DoPlace

    protected:
        void resize(int w, int h);

        int width() const;

        int height() const;

        void place(const LayoutChar &);

        virtual void internal_layout(const QVector<LayoutChar> &input) = 0;
    };

}