#include "linelayouter.h"

namespace fontium {
    void LineLayouter::internal_layout(const vector<LayoutChar> &chars) {
        int w = 0;
        if (chars.empty()) return;
        int min_y = chars.front().y;
        int max_y = chars.front().y + chars.front().h;
        for (const LayoutChar &c :  chars) {
            w += c.w;
            if (c.y < min_y)
                min_y = c.y;
            if ((c.y + c.h) > max_y)
                max_y = c.y + c.h;
        }
        resize(w, max_y - min_y);
        int x = 0;
        //int y = 0;
        for (const LayoutChar &c : chars) {
            LayoutChar l = c;
            l.x = x;
            l.y = (c.y - min_y);
            place(l);
            x += c.w;
        }
    }
}