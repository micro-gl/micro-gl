#pragma once

#include <microgl/dynamic_array.h>
#include <microgl/vec3.h>
#include <microgl/triangles.h>

template <typename number>
struct model_3d {
    using index = unsigned int;
    using vertex = microgl::vec3<number>;
    model_3d(index triangles, microgl::triangles::indices type, const dynamic_array<vertex> &vertices, const dynamic_array<index> &indices) :
                    triangles{triangles}, type{type}, vertices{vertices}, indices{indices} {}
    index triangles=0;
    microgl::triangles::indices type;
    dynamic_array<vertex> vertices;
    dynamic_array<index> indices;
};
