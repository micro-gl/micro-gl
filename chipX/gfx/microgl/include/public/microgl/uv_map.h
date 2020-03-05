#pragma once

#include <microgl/vec2.h>

namespace microgl {

    template<typename number>
    class uv_map {
    private:
        using const_ref = const number &;
        using vertex = vec2<number>;

    public:
        static
        auto compute(const vertex *points,
                     const unsigned size,
                     const_ref u0=number(0), const_ref v0=number(0),
                     const_ref u1=number(1), const_ref v1=number(1)) -> vertex * {

            auto * map= new vertex[size];
            number min_x, max_x, min_y, max_y;
            min_x=max_x=points[0].x;
            min_y=max_y=points[0].y;
            // compute bounding box
            for (unsigned ix = 0; ix < size; ++ix) {
                const auto & pt = points[ix];
                if(pt.x < min_x) min_x=pt.x;
                if(pt.y < min_y) min_y=pt.y;
                if(pt.x > max_x) max_x=pt.x;
                if(pt.y > max_y) max_y=pt.y;
            }
            const number f_w = (u1-u0)/(max_x-min_x);
            const number f_h = (v1-v0)/(max_y-min_y);
            for (unsigned ix = 0; ix < size; ++ix) {
                const auto & pt = points[ix];
                map[ix]={u0+(pt.x-min_x)*f_w, v0+(pt.y-min_y)*f_h};
            }

            return map;
        }

    };

}