#pragma once

#include <microgl/vec2.h>

namespace microgl {
    namespace functions {
        using l64=long long;
        l64 orient2d(const vec2<l64> &a, const vec2<l64> &b, const vec2<l64> &c, unsigned char precision) {
            return (l64(b.x-a.x)*(c.y-a.y)>>precision) - (l64(b.y-a.y)*(c.x-a.x)>>precision);
        }

        int orient2d(const vec2<int> &a, const vec2<int> &b, const vec2<int> &c, unsigned char precision) {
            return (l64(b.x-a.x)*(c.y-a.y)>>precision) - (l64(b.y-a.y)*(c.x-a.x)>>precision);
        }

        template<typename number>
        inline number orient2d(const vec2<number> &a, const vec2<number> &b, const vec2<number> &c) {
            return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
        }
    }
}
