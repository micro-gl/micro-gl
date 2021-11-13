/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. If this file is to be used or derived and any of the following conditions apply:
    a. for commercial usage by any company, entity or person, or
    b. by a registered company/entity/corporation for commercial or non-commercial usage,
    Then, a special permission has to be granted by the Author (Tomer Shalev) for that entity.
    As a consequence of the above conditions, ONLY entities which are not companies such as a person,
    or a group who want to use or derive this file for non-commercial usage are permitted without
    being given a special permission by the Author (Tomer Shalev)

 2. In any case of granted permission to use this file according to case 1 (see above), derivative
    work and usage of this file should be credited to the project and the author of this project.

 3. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "vec2.h"

namespace microtess {

    template <typename number> struct half_edge_vertex_t;
    template <typename number> struct half_edge_face_t;
    template <typename number> struct poly_info_t;

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
        microtess::vec2<number> coords{};
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
        const microtess::vec2<number> *points= nullptr;
        unsigned int size=0;
        int id=-1;
        bool is_closed=false;
    };
}
