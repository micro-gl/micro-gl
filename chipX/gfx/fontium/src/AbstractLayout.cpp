#include "AbstractLayout.h"
#include <fontium/LayoutConfig.h>

namespace fontium {
    AbstractLayout::AbstractLayout(const LayoutConfig *config) : m_config{config} {
    }

    LayoutResult &AbstractLayout::layout(const vector<LayoutChar> &input) {
        m_result.placed.clear();
        m_compact_w = 0;
        m_compact_h = 0;

        // todo: consider in-place processing instead od copying
        vector<LayoutChar> input_copied = input;
        {
            // give blocks more area
            for (unsigned ix = 0; ix < input_copied.size(); ix++) {
                if (m_config->one_pixel_offset) {
                    input_copied[ix].w+=1;
                    input_copied[ix].h+=1;
//                    input_copied[ix].x-=1;
//                    input_copied[ix].y-=1;
                }
                input_copied[ix].w += m_config->offset_left + m_config->offset_right;
                input_copied[ix].h += m_config->offset_top + m_config->offset_bottom;
            }
        }
        processing_hook(input_copied);
        internal_layout(input_copied);
        resize(m_compact_w, m_compact_h);
        return m_result;
    }

    void AbstractLayout::processing_hook(QVector<LayoutChar> &) {
    }

    static unsigned int nextpot(unsigned int val) {
        val--;
        val = (val >> 1) | val;
        val = (val >> 2) | val;
        val = (val >> 4) | val;
        val = (val >> 8) | val;
        val = (val >> 16) | val;
        val++;
        return val;
    }

    void AbstractLayout::resize(int w, int h) {
        if (m_config) {
            if (m_config->one_pixel_offset) {
                w += 2;
                h += 2;
            }

            if (m_config->pot_image) {
                w = nextpot(w);
                h = nextpot(h);
            }

            int sizeIncrement = m_config->size_increment;
            if (sizeIncrement > 1) {
                w = ((w + sizeIncrement - 1) / sizeIncrement) * sizeIncrement;
                h = ((h + sizeIncrement - 1) / sizeIncrement) * sizeIncrement;
            }
        }
        m_result.width = w;
        m_result.height = h;
    }

    int AbstractLayout::width() const {
        int w = m_result.width;
        if (m_config->one_pixel_offset)
            w -= 2;
        return w;
    }

    int AbstractLayout::height() const {
        int h = m_result.height;
        if (m_config->one_pixel_offset)
            h -= 2;
        return h;
    }

    void AbstractLayout::place(const LayoutChar &c) {
        LayoutChar out = c;
        if ((out.x + out.w) > m_compact_w)
            m_compact_w = out.x + out.w;
        if ((out.y + out.h) > m_compact_h)
            m_compact_h = out.y + out.h;
        if (m_config) {
            if (m_config->one_pixel_offset) {
                out.x++; // todo: why ?
                out.y++;
                out.w--;
                out.h--;
            }
        }
        m_result.placed.push_back(out);
    }
}