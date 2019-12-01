//#include "planarize_division.h"

#include "planarize_division.h"

namespace tessellation {

    template<typename number>
    auto planarize_division<number>::create_frame(const chunker<vertex> &pieces,
                                                  const static_pool & pool) -> half_edge_face * {
        // find bbox of all
        const auto pieces_length = pieces.size();

        vertex left_top=pieces.raw_data()[0];
        vertex right_bottom=left_top;

        for (int ix = 0; ix < pieces_length; ++ix) {
            auto const piece = pieces[ix];
            const auto piece_size = piece.size;
            for (int jx = 0; jx < piece_size; ++jx) {
                const auto * current_vertex = piece.data[jx];
                if(current_vertex->x<left_top.x)
                    left_top.x = current_vertex->x;
                if(current_vertex->y<left_top.y)
                    left_top.y = current_vertex->y;
                if(current_vertex->x>right_bottom.x)
                    right_bottom.x = current_vertex->x;
                if(current_vertex->y>right_bottom.y)
                    right_bottom.y = current_vertex->y;

            }
        }

        left_top.x -= 10;
        left_top.y -= 10;
        right_bottom.x += 10;
        right_bottom.y += 10;

        auto * v0 = pool.get_vertex();
        auto * v1 = pool.get_vertex();
        auto * v2 = pool.get_vertex();
        auto * v3 = pool.get_vertex();

        v0->coords = left_top;
        v1->coords = {left_top.x, right_bottom.y};
        v2->coords = right_bottom;
        v3->coords = {right_bottom.x, left_top.y};

        // note, no need for twin edges
        auto * edge_0 = pool.get_edge();
        auto * edge_1 = pool.get_edge();
        auto * edge_2 = pool.get_edge();
        auto * edge_3 = pool.get_edge();

        // ccw from left-top vertex
        edge_0->origin = v0;
        edge_1->origin = v1;
        edge_2->origin = v2;
        edge_3->origin = v3;

        v0->edge = edge_0;
        v1->edge = edge_1;
        v2->edge = edge_2;
        v3->edge = edge_3;

        edge_0->next = edge_1;
        edge_1->next = edge_2;
        edge_2->next = edge_3;
        edge_3->next = edge_0;

        edge_0->prev = edge_3;
        edge_1->prev = edge_0;
        edge_2->prev = edge_1;
        edge_3->prev = edge_2;

        auto * face = pool.get_face();
        edge_0->face = face;
        edge_1->face = face;
        edge_2->face = face;
        edge_3->face = face;

        // CCW around face, face is always to the left of the walk
        face->edge = edge_0;

        return face;
        // note: we might find out that edge-face pointer and edge-prev are not required.
    }

    template<typename number>
    auto planarize_division<number>::build_edges(const chunker<vertex> &pieces,
                                                 const static_pool & pool) -> conflict * {

        const auto pieces_length = pieces.size();
        conflict * conflict_first = nullptr;
        conflict * conflict_last = nullptr;

        // build edges and twins, do not make next/prev connections
        for (int ix = 0; ix < pieces_length; ++ix) {
            half_edge * edge_first = nullptr;
            half_edge * edge_last = nullptr;

            auto const piece = pieces[ix];
            const auto piece_size = piece.size;
            for (int jx = 0; jx < piece_size; ++jx) {
                // v, e, c
                auto * v = pool.get_vertex();
                auto * e = pool.get_edge();
                auto * c = pool.get_conflict_node();

                // hook current v to e and c
                v->coords = piece.data[jx];
                e->origin = v;
                c->vertex = v;

                // record first edge of polygon
                if(edge_first==nullptr) {
                    edge_first = e;
                }

                // set last twin
                if(edge_last) {
                    auto * e_last_twin = pool.get_edge();
                    e_last_twin->origin = v;
                    edge_last->twin = e_last_twin;
                    e_last_twin->twin = edge_last;
                }

                if(conflict_last) {
                    conflict_last->next = c;
                } else {
                    conflict_first = c;
                }

                edge_last = e;
                conflict_last = c;
            }

            // hook the last edge, from last vertex into first vertex
            if(edge_last) {
                auto * e_last_twin = pool.get_edge();
                e_last_twin->origin = edge_first->origin;
                edge_last->twin = e_last_twin;
                e_last_twin->twin = edge_last;
            }

        }

        return conflict_first;
    }


    template<typename number>
    void planarize_division<number>::compute(const chunker<vertex> &pieces) {

        const auto v_size = pieces.unchunked_size();
        // plus 4s for the sake of frame
        static_pool static_pool(v_size + 4, 4 + v_size*2, 1, v_size);
        dynamic_pool dynamic_pool{};
        // create the frame
        auto * main_face = create_frame(pieces, static_pool);
        // create edges and conflict lists
        auto * conflicts = build_edges(pieces, *main_face, static_pool);
        // hook conflicts
        main_face->conflict_list = conflicts;
        // here make a permutation
        // todo: take permutation.




    }

    template<typename number>
    bool planarize_division<number>::calcIntersection(const half_edge &l, const half_edge &r,
                                                      float &alpha, float &alpha1) {
        /*
        // endpoints
        auto a = *(this->start());
        auto b = *(this->end());
        auto c = *(l.start());
        auto d = *(l.end());

        // vectors
        auto ab = b - a;
        auto cd = d - c;

        auto dem = ab.x * cd.y - ab.y * cd.x;

        // parallel lines
        // todo:: revisit when thinking about fixed points
        if (abs(dem) < NOISE)
            return IntersectionType::PARALLEL;
        else {
            auto ca = a - c;
            auto ac = -ca;

            alpha = (ca.y * cd.x - ca.x * cd.y) / dem;
            alpha1 = (ab.y * ac.x - ab.x * ac.y) / dem;

            intersection = a + ab * alpha;

            // test for segment intersecting (alpha)
            if ((alpha < 0.0) || (alpha > 1.0))
                return IntersectionType::NO_INTERSECT;
            else {;//if(false){
                float num = ca.y * ab.x - ca.x * ab.y;

                if (dem > 0.0) {
                    if (num < 0.0 || num > dem)
                        return IntersectionType::NO_INTERSECT;
                } else {
                    if (num > 0.0 || num < dem)
                        return IntersectionType::NO_INTERSECT;
                }
            }
        }
        return IntersectionType::INTERSECT;
         */
    }


    /*
    template <typename number>
    void planarize_division<number>::find_intersections(chunker<vertex> &pieces,
                                                        master_intersection_list &master_list,
                                                        dynamic_array<vertex *> &allocated_intersection) {
        // phase 1:: record trivial intersections
        for (unsigned long poly = 0; poly < pieces.size(); ++poly) {
            auto current_chunk = pieces[poly];
            auto *current_list = current_chunk.data;
            const auto size = current_chunk.size;

            for (unsigned long ix = 0; ix < size; ++ix) {
                int ix_next = ix + 1 >= size ? 0 : int(ix + 1);
                int ix_prev = int(ix - 1) < 0 ? int(size - 1) : int(ix - 1);
                vertex *current = &current_list[ix];
                vertex *next = &current_list[ix_next];
                vertex *prev = &current_list[ix_prev];

                segment l1(prev, current);
                segment l2(current, next);

                l1.sortVertices();
                l2.sortVertices();

                master_list.push_back(intersection(current, 2.0, -1.0, l1, l2));
            }
        }

        // phase 2:: find self intersections of each polygon
        for (unsigned long poly = 0; poly < pieces.size(); ++poly) {
            auto current_chunk = pieces[poly];
            auto *current_list = current_chunk.data;
            const auto size = current_chunk.size;

            if (size == 0)
                continue;

            for (unsigned long ix = 0; ix < size - 1; ++ix) {

                segment edge_0{&current_list[ix], &current_list[(ix + 1)]};

                for (unsigned long jx = ix + 1; jx < size; ++jx) {
                    segment edge_1{&current_list[jx],
                                   &current_list[(jx + 1) % size]};

                    vertex intersection_v;
                    float al1, al2;

                    // see if any of the segments have a mutual endpoint
                    if (!edge_1.is_bbox_overlapping_with(edge_0))
                        continue;
                    if (edge_1.has_mutual_endpoint(edge_0))
                        continue;
                    if (edge_1.calcIntersection(edge_0, intersection_v, al1, al2)
                        != segment::IntersectionType::INTERSECT)
                        continue;

                    auto *found_intersection = new vertex(intersection_v);

                    allocated_intersection.push_back(found_intersection);

                    master_list.push_back(intersection(found_intersection,
                                                       al1, al2, edge_1, edge_0));

                }
            }

        }

        // finds the intersection points between every polygon edge

        dynamic_array<segment> edges, edges1;

        for (unsigned long poly1 = 0; poly1 < pieces.size() - 1; ++poly1) {
            auto current_chunk1 = pieces[poly1];
            auto *current_poly1 = current_chunk1.data;
            const unsigned long size = current_chunk1.size;

            edges.clear();
            for (unsigned ix = 0; ix < size; ++ix) {
                unsigned ix_next = ix + 1 >= size ? 0 : ix + 1;
                vertex *current = &current_poly1[ix];
                vertex *next = &current_poly1[ix_next];

                segment edge(current, next);
                edge.sortVertices();
                edges.push_back(edge);
            }

            // sorting is crucial for the integrity of the output,
            // since I am applying an optimization to discard future edges based
            // on sorting
            qsort_s(edges.data(), edges.size(), sizeof(segment),
                    compare_edges, nullptr);

            for (unsigned long poly2 = poly1 + 1; poly2 < pieces.size(); ++poly2) {

                edges1.clear();
                auto current_chunk2 = pieces[poly2];
                auto *current_poly2 = current_chunk2.data;
                const unsigned long size2 = current_chunk2.size;

                for (unsigned ix = 0; ix < size2; ++ix) {
                    unsigned ix_next = ix + 1 >= size2 ? 0 : ix + 1;
                    vertex *current = &current_poly2[ix];
                    vertex *next = &current_poly2[ix_next];

                    segment edge(current, next);
                    edge.sortVertices();
                    edges1.push_back(edge);
                }

                qsort_s(edges1.data(), edges1.size(), sizeof(segment),
                        compare_edges, nullptr);

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
                            != segment::IntersectionType::INTERSECT)
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
*/
}