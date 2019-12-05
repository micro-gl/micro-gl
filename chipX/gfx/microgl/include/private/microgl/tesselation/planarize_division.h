#pragma once

#include <microgl/tesselation/half_edge.h>
#include <microgl/chunker.h>

namespace tessellation {
#define abs(a) ((a)<0 ? -(a) : (a))

    template <typename number>
    class planarize_division {
    private:

    public:
        using vertex = microgl::vec2<number>;
        using index = unsigned int;
        using half_edge = half_edge_t<number>;
        using half_edge_vertex = half_edge_vertex_t<number>;
        using half_edge_face = half_edge_face_t<number>;
        using conflict = conflict_node_t<number>;

        struct static_pool {
        private:
            half_edge_vertex * _vertices = nullptr;
            half_edge * _edges = nullptr;
            half_edge_face * _faces = nullptr;
            conflict * _conflicts = nullptr;
            index _curr_v = 0;
            index _curr_e = 0;
            index _curr_f = 0;
            index _curr_c = 0;

        public:
            half_edge_vertex * get_vertex() {
                return &_vertices[_curr_v++];
            }
            half_edge * get_edge() {
                return &_edges[_curr_e++];
            }
            half_edge_face * get_face() {
                return &_faces[_curr_f++];
            }
            conflict * get_conflict_node() {
                return &_conflicts[_curr_c++];
            }

            static_pool(const int v, const int e, const int f, const int c) {
                _vertices = new half_edge_vertex[v];
                _edges = new half_edge[e];
                _faces = new half_edge_face[f];
                _conflicts = new conflict[c];
            }

            ~static_pool() {
                delete [] _vertices;
                delete [] _edges;
                delete [] _faces;
                delete [] _conflicts;

                _vertices = nullptr;
                _edges = nullptr;
                _faces = nullptr;
                _conflicts = nullptr;
            }
        };

        struct dynamic_pool {
        private:
            dynamic_array<half_edge_vertex *> _vertices;
            dynamic_array<half_edge *> _edges;
            dynamic_array<half_edge_face *> _faces;

        public:
            ~dynamic_pool() {
                const auto size_v = _vertices.size();
                const auto size_e = _edges.size();
                const auto size_f = _faces.size();
                for (int ix = 0; ix < size_v; ++ix)
                    delete _vertices.data()[ix];
                for (int ix = 0; ix < size_e; ++ix)
                    delete _edges.data()[ix];
                for (int ix = 0; ix < size_f; ++ix)
                    delete _faces.data()[ix];
            };

            auto create_vertex(const vertex &coords) -> half_edge_vertex * {
                auto * v = new half_edge_vertex();
                v->coords = coords;
                _vertices.push_back(v);
                return v;
            }

            auto create_edge() -> half_edge * {
                auto * v = new half_edge();
                _edges.push_back(v);
                return v;
            }

            auto create_face() -> half_edge_face * {
                auto * v = new half_edge_face();
                _faces.push_back(v);
                return v;
            }

        };

        struct trapeze {
//            half_edge_vertex *left_top=nullptr;
//            half_edge_vertex *left_bottom=nullptr;
//            half_edge_vertex *right_bottom=nullptr;
//            half_edge_vertex *right_top=nullptr;
            // in ccw order
            half_edge *left_top=nullptr;
            half_edge *left_bottom=nullptr;
            half_edge *right_bottom=nullptr;
            half_edge *right_top=nullptr;
        };

        struct split_result {
            half_edge_face * face;
            half_edge * edge;
        };

        enum class point_with_trapeze {
            inside, left_wall, right_wall, top_wall, bottom_wall, boundaty_vertex
//            left_top_vertex,left_bottom_vertex,right_bottom_vertex,right_top_vertex
        };
        enum class intersection {
            intersect, none, parallel
        };

        static
        void compute(const chunker<vertex> & pieces);

//    private:

        static
        auto create_frame(const chunker<vertex> &pieces, static_pool & pool) -> half_edge_face *;

        static
        auto build_edges_and_conflicts(const chunker<vertex> &pieces, half_edge_face & main_frame, static_pool & pool) -> half_edge** ;

        static
        void insert_edge(half_edge *edge, dynamic_pool &pool);

        static
        int classify_point(const vertex &point, const vertex &a, const vertex &b);

        static
        auto split_intermediate_face_with_edge(half_edge_face *face, half_edge *edge, dynamic_pool &pool) -> half_edge *;

        static
        bool segment_intersection_test(const vertex &a, const vertex &b,
                                       const vertex &c, const vertex &d,
                                       vertex &intersection,
                                       float &alpha, float &alpha1);

        static
        auto infer_trapeze(const half_edge_face *face) -> trapeze;

        // handle the starting face to create an intermediate face
        static
        auto handle_start_face(half_edge_face *face, half_edge *edge, dynamic_pool &pool) -> split_result;

        static
        point_with_trapeze classify_point_with_trapeze(const vertex &point, const trapeze &trapeze);

        static
        auto split_edge_at(half_edge_vertex *point, half_edge *edge, dynamic_pool &pool) -> half_edge * ;

        static
        half_edge *
        try_insert_vertex_on_trapeze_boundary_at(const vertex &v, const trapeze &trapeze,
                                                 point_with_trapeze where_boundary, dynamic_pool &pool);

        static
        number evaluate_line_at_x(number x, const vertex &a, const vertex &b);

        static
        void
        insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge, half_edge *vertex_b_edge, dynamic_pool &dynamic_pool);
    };


}

#include "planarize_division.cpp"