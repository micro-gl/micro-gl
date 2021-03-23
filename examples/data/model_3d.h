#pragma once

#include <microgl/dynamic_array.h>
#include <microgl/vec2.h>
#include <microgl/vec3.h>
#include <microgl/triangles.h>
#include <initializer_list>

template <typename number>
struct model_3d {
    using index = unsigned int;
    using vertex2 = microgl::vec2<number>;
    using vertex3 = microgl::vec3<number>;
    model_3d(index triangles, microgl::triangles::indices type,
             const dynamic_array<vertex3> &vertices,
             const dynamic_array<vertex2> &uvs,
             const dynamic_array<index> &indices) :
            triangles{triangles}, type{type}, vertices{vertices}, uvs{uvs}, indices{indices} {}

    model_3d(index triangles, microgl::triangles::indices type,
             const std::initializer_list<vertex3> &vertices,
             const std::initializer_list<vertex2> &uvs,
             const std::initializer_list<index> &indices) :
            triangles{triangles}, type{type}, vertices{vertices}, uvs{uvs}, indices{indices} {}

    index triangles=0;
    microgl::triangles::indices type;
    dynamic_array<vertex3> vertices;
    dynamic_array<vertex2> uvs;
    dynamic_array<index> indices;
};
