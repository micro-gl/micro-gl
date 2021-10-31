#pragma once

#include "microgl/tesselation/vec2.h"

namespace microgl {

    template<typename number1, typename number2=number1>
    class uv_map {
    private:
        using const_ref = const number1 &;
        using vertex = vec2<number1>;
        using vertex_uv = vec2<number2>;

    public:
        static
        auto compute(const vertex *points,
                     const unsigned size,
                     const_ref u0=number2(0), const_ref v0=number2(1),
                     const_ref u1=number2(1), const_ref v1=number2(0)) -> vertex_uv * {

            auto * map= new vertex[size];
            number1 min_x, max_x, min_y, max_y;
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
            //const number2 f_w = (u1-u0)/number2(max_x-min_x);
            //const number2 f_h = (v1-v0)/number2(max_y-min_y);
            for (unsigned ix = 0; ix < size; ++ix) {
                const auto & pt = points[ix];
                number2 u = u0+(number2(pt.x-min_x)*(u1-u0))/number2(max_x-min_x);
                number2 v = v0+(number2(pt.y-min_y)*(v1-v0))/number2(max_y-min_y);
                map[ix]={u, v};
            }

            return map;
        }

    };

}