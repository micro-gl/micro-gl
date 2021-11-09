/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
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