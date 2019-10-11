#pragma once

namespace microgl {
    namespace polygons {

        enum class hints {
            // non self intersecting polygon, can be convex or concave
                    SIMPLE,
            // convex polygon
                    CONVEX,
            // concave polygon
                    CONCAVE,
            // self intersecting polygon
                    NON_SIMPLE,
                    COMPLEX,
                    SELF_INTERSECTING,
        };

    }

}