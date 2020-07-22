#include "boxlayouter.h"

#include <cmath>

namespace fontium {
    struct Line {
        int min_y;
        int max_y;
        int y;

        Line() : min_y(0), max_y(0), y(0) {}

        explicit Line(const LayoutChar &c) : y(0) {
            min_y = c.y;
            max_y = c.y + c.h;
            //chars.push_back(&c);
        }

        int h() const { return max_y - min_y; }

        void append(const LayoutChar &c) {
            if (c.y < min_y)
                min_y = c.y;
            if ((c.y + c.h) > max_y)
                max_y = c.y + c.h;
            chars.push_back(&c);
        }

        QVector<const LayoutChar *> chars;
    };

    void BoxLayouter::internal_layout(const vector<LayoutChar> &chars) {
        int h = 0;
        int w = 0;
        if (chars.empty()) return;

        /// speed up
        int area = 0;
        for (const LayoutChar &c : chars)
            area += c.w * c.h;
        int dim = ::sqrt(area);

        resize(dim, dim);
        w = width();
        h = height();


        QVector<Line> lines;

        bool iteration = true;
        while (iteration) {
            int x = 0;
            lines.clear();
            lines.push_back(Line(chars.front()));
            iteration = false;
            for (const LayoutChar &c: chars) {

                if ((x + c.w) > w) {
                    x = 0;
                    int y = lines.back().y;
                    int h = lines.back().h();
                    lines.push_back(Line(c));
                    lines.back().y = y + h;
                }

                if ((lines.back().y + c.h) > h) {
                    if (w > h) {
                        resize(width(), lines.back().y + c.h);
                        h = height();
                    } else {
                        resize(width() + c.w, height());
                        w = width();
                    }
                    iteration = true;
                    break;
                }

                /// place
                lines.back().append(c);
                x += c.w;
            }
        }

        w = width();
        h = height();
        int x = 0;
        for (const Line &line: lines) {
            x = 0;
            for (const LayoutChar *c : line.chars) {
                LayoutChar l = *c;
                l.x = x;
                l.y = line.y + (c->y - line.min_y);
                place(l);
                x += c->w;
            }

        }
    }

}