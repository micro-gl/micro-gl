#pragma once

#include <microgl/vec2.h>

namespace microgl {
    namespace tessellation {

        template <typename number>
        struct half_edge_vertex_t;
        template <typename number>
        struct half_edge_face_t;
        template <typename number>
        struct poly_info_t;

        template <typename number>
        struct half_edge_t {
            half_edge_t * twin = nullptr;
            half_edge_t * next = nullptr;
            half_edge_t * prev = nullptr;
            // origin vertex
            half_edge_vertex_t<number> * origin = nullptr;
            // left incident face
            half_edge_face_t<number> * face = nullptr;
            int winding = 0;
        };

        template <typename number>
        struct half_edge_vertex_t {
            microgl::vec2<number> coords;
            // pointer to any edge that has this vertex as starting point
            half_edge_t<number> * edge = nullptr;
            // used to locate inside the output vertices buffer
            int id=-1;
            int head_id=-1;
            int tess_index=-1;
            bool internal_tess_clipped=false;
        };

        template <typename number>
        struct conflict_node_t {
            poly_info_t<number> *unadded_input_poly=nullptr;
            conflict_node_t * next = nullptr;
        };

        template <typename number>
        struct half_edge_face_t {
            // any edge of the face
            half_edge_t<number> * edge = nullptr;
            // list of conflicting vertices
            conflict_node_t<number> * conflict_list = nullptr;
            int index=0;
            int winding=0;
            bool computed_winding=false;
            bool isValid() { //when deleted for example
                return edge!=nullptr;
            }
        };

        template <typename number>
        struct poly_info_t {
            half_edge_face_t<number> *conflict_face = nullptr;
            const microgl::vec2<number> *points= nullptr;
            index size=0;
            int id=-1;
            bool is_closed=false;
        };


    }
}
