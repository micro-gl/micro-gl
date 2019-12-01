#pragma once

#include <microgl/tesselation/half_edge.h>
#include <microgl/chunker.h>

namespace tessellation {

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
                return _vertices[_curr_v++];
            }
            half_edge * get_edge() {
                return _edges[_curr_e++];
            }
            half_edge * get_face() {
                return _faces[_curr_f++];
            }
            conflict * get_conflict_node() {
                return _conflicts[_curr_c++];
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

                _vertices = _edges = _faces = _conflicts = nullptr;
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

            auto create_vertex() -> half_edge_vertex * {
                auto * v = new half_edge_vertex();
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

        static
        void compute(const chunker<vertex> & pieces);

//    private:

        static
        bool calcIntersection(const half_edge &l, const half_edge &r, float &alpha, float &alpha1);

        static
        auto create_frame(const chunker<vertex> &pieces, const static_pool & pool) -> half_edge_face *;

        static
        auto build_edges(const chunker<vertex> &pieces, const static_pool & pool) -> conflict * ;

    };


}

#include "planarize_division.cpp"