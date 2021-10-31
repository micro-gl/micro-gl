#pragma once

namespace microtess {
    namespace polygons {

        enum class hints {
            // non self intersecting polygon, can be convex or concave, 
            // may have edge points touch other edges interior,
            // this can be done in O(n^2)
            SIMPLE,
            CONCAVE,
            // axis monotone polygons, they are perfect for tesselating 2d function graphs,
            // this can be done in linear O(n) time
            X_MONOTONE,
            Y_MONOTONE,
            // convex polygon, this can be done in linear O(n) time
            CONVEX,
            // self intersecting polygon, this can be done between O(n*log(n)) to O(n^2)
            NON_SIMPLE,
            COMPLEX,
            MULTIPLE_POLYGONS,
            SELF_INTERSECTING,
        };

    }

}