#pragma once

#include <microgl/chunker.h>
#include <microgl/vec2.h>

namespace tessellation {


    template <typename number>
    class simplify_components {
    private:
//        using vertex = microgl::vec2_f;
        using vertex = microgl::vec2<number>;

        struct segment
        {
            // used for determining limit of parallel lines
            constexpr static const float NOISE = 1e-5f;;

            vertex *vertex0= nullptr, *vertex1= nullptr;
            // have vertices been swapped during sortVertices?
            bool m_swappedVertices=false;

            segment()= default;

            enum class IntersectionType
            {
                PARALLEL, NO_INTERSECT, INTERSECT
            };

            enum class bbox_axis {
                overlaps, start_of, end_of,
            };

            vertex * start();
            vertex * end();
            vertex * start() const;
            vertex * end() const;

            // Sets bounding box of line segment to (vertex0.x, ymin, vertex1.x, ymax)
            void sortVertices();
            segment(vertex *vtx0, vertex *vtx1);

            IntersectionType calcIntersection(const segment &l, vertex &intersection,
                                              float &alpha, float &alpha1);

            bool is_bbox_overlapping_with(const segment &a);
            bbox_axis classify_horizontal(const segment &a);
            bbox_axis classify_vertical(const segment &a);
            bbox_axis classify_aligned_segment_relative_to(const segment &a, bool compare_x);
            // this might be done with just pointers compare instead ?
            bool has_mutual_endpoint(const segment &a);
        };

        struct intersection
        {
            vertex *v{};
            float param1{},param2{};
            int index1{}, index2{};
            segment l1{}, l2{};

            vertex * origin1();
            vertex * origin2();
            intersection()= default;
            intersection (vertex *vtx, float p1, float p2, const segment &li, const segment &lj);
            bool isDeadEnd();
        };

        struct edge_vertex
        {
            vertex *v=nullptr;
            float param=0;
            int index=-1;
            edge_vertex()=default;
            edge_vertex (vertex *vtx, float p, int i);
        };

        struct edge
        {
            dynamic_array<edge_vertex> vertices;
            edge() = default;
        };

        // edge_list - list of polygon edges
        using edge_list = dynamic_array<edge>;
        // master intersection list
        using master_intersection_list = dynamic_array<intersection>;
    public:
        using index = unsigned int;

        static
        void compute(chunker<vertex> & pieces,
                     chunker<vertex> & pieces_result);

    private:
        static void next_perturbation(vertex &point);

        static
        void compute_master_list(chunker<vertex> & pieces,
                                 master_intersection_list &master_list,
                                 dynamic_array<vertex *> &allocated_intersection);

        static
        void polygonPartition(chunker<vertex> &result,
                              master_intersection_list &master_list);

#define abs(a) ((a)<0 ? -(a) : (a))

        static
        void fillAddress(master_intersection_list &master_list);

        static
        void findIntersections(chunker<vertex> & pieces,
                               master_intersection_list &master_list,
                               dynamic_array<vertex *> &allocated_intersection);

        static
        bool compare_vertices(const vertex &v1, const vertex &v2);

        static
        int compare_vertices_int(const vertex &v1, const vertex &v2);

        static
        int compare_edge_vertices(const void *a, const void *b, void *ctx);

        static
        int compare_edges(const void *a, const void *b, void *ctx);
    };

}

#include "../../src/simplify_components.cpp"
