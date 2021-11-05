#pragma once

#include "microgl/tesselation/dynamic_array.h"
#include "microgl/math/vertex2.h"
#include "microgl/math/vertex3.h"
#include <microgl/tesselation/triangles.h>
#include <initializer_list>

template <typename number>
struct model_3d {
    using index = unsigned int;
    using vertex2 = microgl::vertex2<number>;
    using vertex3 = microgl::vertex3<number>;
    model_3d(index triangles, microtess::triangles::indices type,
             const dynamic_array<vertex3> &vertices,
             const dynamic_array<vertex2> &uvs,
             const dynamic_array<index> &indices) :
            triangles{triangles}, type{type}, vertices{vertices}, uvs{uvs}, indices{indices} {}

    model_3d(index triangles, microtess::triangles::indices type,
             const std::initializer_list<vertex3> &vertices,
             const std::initializer_list<vertex2> &uvs,
             const std::initializer_list<index> &indices) :
            triangles{triangles}, type{type}, vertices{vertices}, uvs{uvs}, indices{indices} {}

    index triangles=0;
    microtess::triangles::indices type;
    dynamic_array<vertex3> vertices;
    dynamic_array<vertex2> uvs;
    dynamic_array<index> indices;
};
