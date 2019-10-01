// Multipolyon.h
// Written by T. Hain. Extended by L. Subramaniam, Spring, 2003
#pragma once

#include <algorithm>
#include <microgl/dynamic_array.h>
#include <microgl/chunker.h>
#include <microgl/vec2.h>
using namespace std;

namespace tessellation {

    using vertex = microgl::vec2_f;

    enum class direction {
        CW, CCW
    };

    struct segment
    {
        // used for determining limit of parallel lines
        constexpr static const float NOISE = 1e-5f;;

        vertex *vertex0= nullptr, *vertex1= nullptr;
        // have vertices been swapped during sortVertices?
        bool m_swappedVertices=false;

        segment()= default;

        typedef enum
        {
            // lines are parallel within tolerance level
            PARALLEL,
            // lines segments don't intersect
            NO_INTERSECT,
            // line segments intersect
            INTERSECT
        } IntersectionType;

        enum class bbox_axis {
            overlaps,
            start_of,
            end_of,
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
        // order by left edge of bounding box
        bool operator< (const segment &ls) const;
    };

    struct intersection
    {
        vertex *v{};
        float param1{},param2{};
        int index1{}, index2{};
        segment l1, l2;

        vertex * origin1();
        vertex * origin2();
        intersection()= default;
        intersection (vertex *vtx, float p1, float p2, const segment &li, const segment &lj);
        bool operator< (const intersection &i) const;
        bool isDeadEnd();
        bool isPolygonVertex() {
            return v==origin1() || v==origin2();
        }
    };

    struct edge_vertex
    {
        vertex *v;
        float param;
        int index;
        edge_vertex()=default;
        edge_vertex (vertex *vtx, float p, int i);
        bool operator< (const edge_vertex &n) const;
    };

    struct edge
    {
        dynamic_array<edge_vertex> vertices;
        edge() = default;
        bool operator< (const edge &s) const;
    };

    // edge_list - list of polygon edges
    using edge_list = dynamic_array<edge>;
    // master intersection list
    using master_intersection_list = dynamic_array<intersection>;

    class simplify_components {
    public:
        using index = unsigned int;

        static
        void compute(chunker<vertex> & pieces,
                     chunker<vertex> & pieces_result,
                     dynamic_array<direction> &directions) {
            master_intersection_list master_list;
            dynamic_array<vertex *> allocated_intersection;

            // compute the intersection master list
            compute_master_list(pieces, master_list, allocated_intersection);

            // now, we have a complete master list, we can traverse it for polygons.
            polygonPartition(pieces_result, master_list, directions);

            // delete allocated intersections.
            for (unsigned long ix = 0; ix < allocated_intersection.size(); ++ix) {
                delete allocated_intersection[ix];
            }
        }

    private:
        static
        void compute_master_list(chunker<vertex> & pieces,
                                 master_intersection_list &master_list,
                                 dynamic_array<vertex *> &allocated_intersection) {
            edge_list edges;

            // phase 1:: fill the edges structure and initial intersections
            // in the master list
            for (unsigned long poly = 0; poly < pieces.size(); ++poly) {
                auto current_chunk = pieces[poly];
                auto * current_list = current_chunk.data;
                const auto size = current_chunk.size;

                for (unsigned long ix = 0; ix < size; ++ix) {
                    int ix_next = ix+1 >= size ? 0 : int(ix+1);
                    int ix_prev = int(ix-1) < 0 ? int(size-1) : int(ix-1);
                    vertex * current = &current_list[ix];
                    vertex * next = &current_list[ix_next];
                    vertex * prev = &current_list[ix_prev];

                    segment l1(prev, current);
                    segment l2(current, next);
                    edge edge;
                    l1.sortVertices();
                    l2.sortVertices();

                    edge_vertex i1(current, 0, -1);
                    edge_vertex i2(next, 2, -1);
                    // report vertex as a vertex intersection in the master list
                    master_list.push_back(intersection(current, 1, 0, l1, l2 ));
                    // first element is the edge vertex
                    edge.vertices.push_back(i1);
                    // last vertex of the edge
                    edge.vertices.push_back(i2);

                    edges.push_back( edge );
                }
            }

            // phase 3:: find intersections of polygons among other polygons and
            // add the intersections into the master list, this uses bounding boxes optimizations
            findIntersections(pieces, master_list, allocated_intersection);
            // use the edge structure to fill the master list
            fillAddress(edges, master_list);
        }

        static
        void polygonPartition(chunker<vertex> &result,
                              master_intersection_list &master_list,
                              dynamic_array<direction> &directions) {

            dynamic_array<int> stack;

            do
            {
                int current_index = 0;
                bool found_direction = false;
                direction current_direction;
                // remove top intersections that have been
                // completely visited or are dead ends
                while (!stack.empty()) {
                    int idx = stack.back();
                    if (master_list[idx].isDeadEnd())
                        stack.pop_back();
                    else
                        break;
                }

                // find the next intersection to process. If we cant find from
                // the top of the stack (spawned sub-component) then find in
                // the master list, this is HOW we jump between disjoint components !!!
                if (!stack.empty() && stack.back() > 0)
                    current_index = stack.back();
                else {
                    current_index=0;
                    while(master_list[current_index].isDeadEnd() &&
                          (current_index < master_list.size()))
                        current_index++;
                }

                // if we reached the last intersection, we are done
                // else, this is out current index for work
                if (current_index >= master_list.size() - 1 )
                    break;

                auto *current_intersection = &master_list[current_index];
                auto *prev_intersection = &master_list[current_index];

                // start index marks a beginning of a component ?
                const int startIndex = current_index;
                int firstIndex = startIndex;

                result.push_back(*current_intersection->v);
                stack.push_back(current_index);

                // walk the polygon until we get back to the start point,
                // on the way, push potential companion vertices into the stack
                do {
                    // find the next vertex to visit and tag visited companion vertices
                    if (current_intersection->isDeadEnd())
                        current_index++;
                    else
                    {

                        if ((current_intersection->origin1() == current_intersection->v) &&
                            (current_intersection->index2 == -1))
                        {
                            int tempIndex = current_intersection->index1;
                            prev_intersection = current_intersection;
                            current_intersection->index1 = -1;
                            current_index = tempIndex;
                        }
                        else
                        {
                            if ( ((current_intersection->origin1() == prev_intersection->origin1())
                                  ||(current_intersection->origin1() == prev_intersection->origin2() ))
                                 && (current_intersection->index2 != -1 ))
                            {
                                int tempIndex = current_intersection->index2;
                                prev_intersection = current_intersection;
                                current_intersection->index2 = -1;
                                current_index = tempIndex;
                            }
                            else
                            {
                                int tempIndex = current_intersection->index1;
                                prev_intersection = current_intersection;
                                current_intersection->index1 = -1;
                                current_index = tempIndex;
                            }
                        }
                    }

                    if (current_index == -1)
                        continue;

                    current_intersection = &master_list[current_index];

                    result.push_back(*current_intersection->v);
                    stack.push_back(current_index);

                    // this part is only for resolving directions
                    // this part of code relies on the fact that the
                    // vertices are sorted to infer the left-most vertex etc..
                    if (found_direction)
                        continue;
                    // this is a mechanism to get the next vertex, when we get into
                    // direction calculation.
                    if (firstIndex == startIndex)
                        firstIndex = current_index;
                    else
                    {
                        found_direction = true;

                        // tests if polygon vertex
                        if (master_list[startIndex].isPolygonVertex()) {
                            auto win = xProd(*master_list[startIndex].origin1(),
                                         *master_list[startIndex].v,
                                         *master_list[firstIndex].v);

                            current_direction = win>0 ? direction::CW : direction::CCW ;
                        }
                        else {
                            vertex *vtx ;
                            if (master_list[startIndex].index1 == -1)
                                vtx = master_list[startIndex].origin1();
                            else
                                vtx = master_list[startIndex].origin2();

                            auto win = xProd(*vtx,
                                    *master_list[startIndex].v,
                                    *master_list[firstIndex].v);

                            current_direction = win>0 ? direction::CW : direction::CCW ;
                        }

                    }

                } while ((current_index != startIndex) && (current_index != -1)) ;

                directions.push_back(current_direction);
                result.cut_chunk();

            } while (true);

        }

        static
        void fillAddress(edge_list &edges, master_intersection_list &master_list)
        {
            // sort the polygons edges and master list
            sort(edges.data(), edges.data() + edges.size());
            sort(master_list.data(), master_list.data() + master_list.size());

            // push real intersection into the polygon edges lists, for each edge push
            // it's intersecting vertex
            for (unsigned long ix =0; ix < master_list.size() ; ix++ ) {
                auto & intersection = master_list[ix];

                // todo:: break it into more cases to support point on an edge
                if (!intersection.isPolygonVertex()) {
//                if ((intersection.param1 != 0 ) && (intersection.param2 != 0 )) {
                    int i11 = 0, i21 = 0;
                    vertex *searchVertex1 = intersection.origin1();
                    vertex *searchVertex2 = intersection.origin2();

                    // zero index is always a concrete polygon vertex ,
                    // but always concrete part of the polygon, thus can be identified with
                    // unique id
                    while (edges[i11].vertices[0].v != searchVertex1)
                        i11++;
                    while (edges[i21].vertices[0].v != searchVertex2)
                        i21++;

                    edge_vertex Inter1(intersection.v, intersection.param1, int(ix));
                    edge_vertex Inter2(intersection.v, intersection.param2, int(ix));

                    edges[i11].vertices.push_back(Inter1);
                    edges[i21].vertices.push_back(Inter2);
                }
                else {
                    // natural polygon vertex joints can be filled with indices already
                    // this are the first vertex and last vertex, that define an edge of a polygon.
                    // first vertex is the start vertex, and second one is the end vertex.
                    // other vertices are in between, later on we will sort them
                    int vertexIndex = 0;
                    vertex *vertex = intersection.v;

                    while (edges[vertexIndex].vertices[0].v != vertex )
                        vertexIndex++;

                    edges[vertexIndex].vertices[0].index = int(ix);

                    vertexIndex = 0;
                    while (edges[vertexIndex].vertices[1].v != vertex )
                        vertexIndex++;

                    edges[vertexIndex].vertices[1].index = int(ix);
                }

            }

            // sort ONLY the intersections of the poly edge array
            for (unsigned long ix = 0; ix < edges.size(); ix++) {
                auto & edge_vertices = edges[ix].vertices;
                sort(edge_vertices.data(), edge_vertices.data() + edge_vertices.size());
            }

            // setting up the indices for the intersections in the master_list,
            for (unsigned long edge = 0; edge < edges.size(); edge++) {
                // go over the intersections of each edge
                auto & edge_vertices = edges[edge].vertices;
                for (unsigned long ix = 0; ix < edge_vertices.size()-1; ++ix)
                {
                    auto *first_vertex_of_edge = edge_vertices[0].v;
                    int next_index = edge_vertices[ix+1].index;
                    auto & intersection = master_list[edge_vertices[ix].index];

                    // this is actually very simple. each edge is broken/ represented as
                    // segments *--*--*-*----* etc..
                    // for each point on the edge, update the master list where do we
                    // go next.
                    if (intersection.origin2() == first_vertex_of_edge)
                        intersection.index2 = next_index;
                    else// if (intersection.origin1() == first_vertex_of_edge)
                        intersection.index1 = next_index;
//                    else if (intersection.v == first_vertex_of_edge)
//                        intersection.index1 = next_index;
                }
            }

        }

        static
        void findIntersections(chunker<vertex> & pieces,
                               master_intersection_list &master_list,
                               dynamic_array<vertex *> &allocated_intersection)
        {
            // phase 2:: find self intersections of each polygon
            for (unsigned long poly = 0; poly < pieces.size(); ++poly)
            {
                auto current_chunk = pieces[poly];
                auto * current_list = current_chunk.data;
                const auto size = current_chunk.size;

                if(size==0)
                    continue;

                for (unsigned long ix = 0; ix < size-1; ++ix) {

                    segment edge_0{&current_list[ix], &current_list[(ix + 1)]};

                    for (unsigned long jx = ix+1; jx < size; ++jx) {
                        segment edge_1{&current_list[jx],
                                       &current_list[(jx+1)%size]};

                        vertex intersection_v;
                        float al1, al2;

                        // see if any of the segments have a mutual endpoint
                        if(!edge_1.is_bbox_overlapping_with(edge_0))
                            continue;
                        if (edge_1.has_mutual_endpoint(edge_0))
                            continue;
                        if(edge_1.calcIntersection(edge_0, intersection_v, al1, al2)
                           != segment::INTERSECT)
                            continue;

                        auto * found_intersection = new vertex(intersection_v);

                        allocated_intersection.push_back(found_intersection);

                        master_list.push_back(intersection(found_intersection,
                                                           al1, al2, edge_1, edge_0));

                    }
                }

            }

            // finds the intersection points between every polygon edge

            dynamic_array<segment> edges, edges1;

            for (unsigned long poly1 = 0; poly1 < pieces.size()-1; ++poly1) {
                auto current_chunk1 = pieces[poly1];
                auto * current_poly1 = current_chunk1.data;
                const unsigned long size = current_chunk1.size;

                edges.clear();
                for (unsigned ix = 0; ix < size; ++ix) {
                    unsigned ix_next = ix+1 >= size ? 0 : ix+1;
                    vertex * current = &current_poly1[ix];
                    vertex * next = &current_poly1[ix_next];

                    segment edge(current, next);
                    edge.sortVertices();
                    edges.push_back(edge);
                }

                sort(edges.data(), edges.data() + edges.size());

                for (unsigned long poly2 = poly1+1; poly2 < pieces.size(); ++poly2) {

                    edges1.clear();
                    auto current_chunk2 = pieces[poly2];
                    auto * current_poly2 = current_chunk2.data;
                    const unsigned long size2 = current_chunk2.size;

                    for (unsigned ix = 0; ix < size2; ++ix)
                    {
                        unsigned ix_next = ix+1 >= size2 ? 0 : ix+1;
                        vertex * current = &current_poly2[ix];
                        vertex * next = &current_poly2[ix_next];

                        segment edge(current, next);
                        edge.sortVertices();
                        edges1.push_back(edge);
                    }

                    sort(edges1.data(), edges1.data() + edges1.size());

                    unsigned nEdges = edges.size();
                    unsigned nEdges1 = edges1.size();

                    if (nEdges < 3)
                        continue;

                    for (unsigned i = 0; i < nEdges; ++i) {
                        auto &edge_0 = edges[i];

                        for (unsigned j = 0; j < nEdges1; ++j) {

                            auto &edge_1 = edges1[j];

                            // if edge_1 is completely to the right of edge_0, then no intersection
                            // occurs, and also, since edge_1 syblings are sorted on the x-axis,
                            // we can skip it's upcoming syblings, therefore we break;
                            auto h_classify = edge_1.classify_horizontal(edge_0);
                            if (h_classify == segment::bbox_axis::end_of)
                                break;
                            else if (h_classify != segment::bbox_axis::overlaps)
                                continue;
                            else {
                                // we have horizontal overlap, let's test for vertical overlap
                                if (edge_1.classify_vertical(edge_0) != segment::bbox_axis::overlaps)
                                    continue;
                            }

                            // see if any of the segments have a mutual endpoint
                            if (edge_1.has_mutual_endpoint(edge_0))
                                continue;

                            vertex intersection_v;
                            float param1, param2;

                            // test and compute intersection
                            if (edge_0.calcIntersection(edge_1, intersection_v, param1, param2)
                                != segment::INTERSECT)
                                continue;

                            auto *found_intersection = new vertex(intersection_v);

                            allocated_intersection.push_back(found_intersection);

                            master_list.push_back(intersection(found_intersection,
                                                               param1, param2,
                                                               edge_0, edge_1));

                        }


                    }

                }

            }

        }

        static
        float xProd(const vertex &p, vertex &q, vertex &r) {
            return q.x*(r.y - p.y) + p.x*(q.y - r.y) + r.x*(p.y - q.y);
        }

    };

}
