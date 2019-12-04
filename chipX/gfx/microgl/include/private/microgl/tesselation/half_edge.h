#pragma once

#include <microgl/vec2.h>

namespace tessellation {

    enum class point_type {
        regular, attachment, intersection, unknown
    };

    enum class edge_type {
        vertical, horizontal, unknown
    };

    template <typename number>
    struct half_edge_vertex_t;
    template <typename number>
    struct half_edge_face_t;

    template <typename number>
    struct conflict_node_t {
        // conflicting vertex
        half_edge_vertex_t<number> *vertex = nullptr;
        // next one
        conflict_node_t * next = nullptr;
    };

    template <typename number>
    struct half_edge_t {
        half_edge_t * twin = nullptr;
        half_edge_t * next = nullptr;
        half_edge_t * prev = nullptr;
        // origin vertex
        half_edge_vertex_t<number> * origin = nullptr;
        // left incident face
        half_edge_face_t<number> * face = nullptr;
        edge_type type = edge_type::unknown;
        int winding = 0;
    };

    template <typename number>
    struct half_edge_vertex_t {
        microgl::vec2<number> coords;
        // pointer to any edge that has this vertex as starting point
        half_edge_t<number> * edge = nullptr;
        // pointer to the conflicting face, valid for regular vertices
        half_edge_face_t<number> *conflict_face = nullptr;
        point_type type = point_type::unknown;
    };

    template <typename number>
    struct half_edge_face_t {
        half_edge_t<number> * edge = nullptr;
        // list of conflicting vertices
        conflict_node_t<number> * conflict_list = nullptr;
    };

}