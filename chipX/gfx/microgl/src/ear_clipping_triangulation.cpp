#include <microgl/tesselation/ear_clipping_triangulation.h>
#include <microgl/qsort.h>

namespace tessellation {

    // the extremal left-bottom most vertex is always a convex vertex
    int find_left_bottom_most_vertex(vertex * poly,
                                     const int size) {
        int index = 0;
        vertex value = poly[0];

        for (int ix = 0; ix < size; ++ix) {
            auto & v = poly[ix];

            if(v.x < value.x) {
                value = v;
                index = ix;
            }
            else if(v.x==value.x) {
                if(v.y > value.y) {
                    value = v;
                    index = ix;
                }
            }

        }

        return index;
    }

    ear_clipping_triangulation::ear_clipping_triangulation(bool DEBUG) :
            _DEBUG{DEBUG} {};

    ear_clipping_triangulation::LinkedList *
    ear_clipping_triangulation::polygon_to_linked_list(vertex *$pts,
                                                       index offset,
                                                       index size
                                                       ) {
        // create a linked list with static memory on the stack :)
        // lets also make heap version
        auto *pts = new LinkedList{true};

        // todo:: filter out degenerate points, points at the same location
        for (index ix = 0; ix < size; ++ix) {
            auto * node = new Node{};
            node->data.pt = &$pts[ix];
            node->data.original_index = offset + ix;

            pts->addLast(node);
        }

        return pts;
    }

    int compare (const void * a, const void * b, void * ctx)
    {
        auto * left_mosts = (vertex *) ctx;
        auto * a_casted = (ear_clipping_triangulation::LinkedList *)a;
        auto * b_casted = (ear_clipping_triangulation::LinkedList *)b;

        vertex &a_lm = left_mosts[a_casted->index];
        vertex &b_lm = left_mosts[b_casted->index];

        bool a_before_b = a_lm.x < b_lm.x || (a_lm.x==b_lm.x && a_lm.y > b_lm.y);

        if(a_before_b)
            return -1;

        if(a_lm==b_lm)
            return 0;

        return 1;
    }


    void ear_clipping_triangulation::compute(vertex *$pts,
                                             index size,
                                             dynamic_array<index> & indices_buffer_triangulation,
                                             dynamic_array<triangles::boundary_info> * boundary_buffer,
                                             dynamic_array<hole> * holes,
                                             dynamic_array<vec2_f> * result,
                                             const triangles::TrianglesIndices &requested) {

        if(holes) {
            auto count = 1 + holes->size();

            vertex left_mosts[count];
            LinkedList *lists[count];
            // we need to populate the lists and sort them by left-most points
            left_mosts[0] = $pts[find_left_bottom_most_vertex($pts, size)];
            lists[0] = polygon_to_linked_list($pts, size, 0);
            lists[0]->index = 0;

            for (index ix = 0; ix < holes->size(); ++ix) {
                auto hole = (*holes)[ix];
                left_mosts[ix + 1] = hole.points[find_left_bottom_most_vertex(hole.points, hole.size)];
                lists[ix + 1] = polygon_to_linked_list(hole.points, hole.offset, hole.size);
                lists[ix + 1]->index = ix + 1;
            }

            qsort_s(lists, count, sizeof(LinkedList), compare, left_mosts);
        }

    }

    void ear_clipping_triangulation::compute(microgl::vec2_32i *$pts,
                                           index size,
                                             dynamic_array<index> & indices_buffer_triangulation,
                                             dynamic_array<triangles::boundary_info> * boundary_buffer,
                                           const triangles::TrianglesIndices &requested) {

        bool requested_triangles_with_boundary =
                requested==triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY;
        auto &indices = indices_buffer_triangulation;

        // create a linked list with static memory on the stack :)
        // lets also make heap version
        LinkedList pts{true};
        Node nodes[size];

        // todo:: filter out degenerate points, points at the same location
//        for (index ix = 0; ix < size; ++ix) {
//            nodes[ix].data = {&$pts[ix], ix};
//
//            pts.addLast(&nodes[ix]);
//        }

        // start algorithm

        index ind = 0;
        Node * point;

        for (index ix = 0; ix < size - 2; ++ix) {

            point = pts.getFirst();

//            for (index jx = 0; jx < size; ++jx) {
            for (index jx = 0; jx < pts.size(); ++jx) {

                if (isDegenrate(point) || (isConvex(point, &pts) && isEmpty(point, &pts))) {

                    indices.push_back(point->predecessor()->data.original_index);
                    indices.push_back(point->data.original_index);
                    indices.push_back(point->successor()->data.original_index);

                    // record boundary
                    if(requested_triangles_with_boundary) {
                        // classify if edges are on boundary
                        unsigned int first_edge_index_distance = abs((int)indices[ind + 0] - (int)indices[ind + 1]);
                        unsigned int second_edge_index_distance = abs((int)indices[ind + 1] - (int)indices[ind + 2]);
                        unsigned int third_edge_index_distance = abs((int)indices[ind + 2] - (int)indices[ind + 0]);

                        bool first = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                        bool second = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                        bool third = third_edge_index_distance==1 || third_edge_index_distance==size-1;

                        index info = triangles::create_boundary_info(first, second, third);

                        boundary_buffer->push_back(info);
                    }

                    ind += 3;

                    // prune the point from the polygon
                    pts.unlink(point);

                    break;
                }

                point = point->next;
            }

        }

        pts.clear();
    }

    long long
    ear_clipping_triangulation::orientation_value(const ear_clipping_triangulation::Node *i,
                                                const ear_clipping_triangulation::Node *j,
                                                const ear_clipping_triangulation::Node *k) {
        return i->data.pt->x * int64_t(j->data.pt->y - k->data.pt->y) +
               j->data.pt->x * int64_t(k->data.pt->y - i->data.pt->y) +
               k->data.pt->x * int64_t(i->data.pt->y - j->data.pt->y);
    }

    // ts
    int ear_clipping_triangulation::neighborhood_orientation_sign(
            const ear_clipping_triangulation::Node *v) {
        const Node * l = v->predecessor();
        const Node * r = v->successor();

        // pay attention that this can return 0, although in the algorithm
        // it does not return 0 never here.
        return sign_orientation_value(l, v, r) > 0 ? 1 : -1;
    }

    // tv
    char
    ear_clipping_triangulation::sign_orientation_value(const ear_clipping_triangulation::Node *i,
                                                     const ear_clipping_triangulation::Node *j,
                                                     const ear_clipping_triangulation::Node *k) {
        auto v = orientation_value(i, j, k);

        // we clip, to avoid overflows down the road
        if(v > 0)
            return 1;
        else if(v < 0)
            return -1;
        else
            return 0;
    }

    ear_clipping_triangulation::Node *ear_clipping_triangulation::maximal_y_element(
                                const ear_clipping_triangulation::LinkedList *list) {
        Node * first = list->getFirst();
        Node * maximal_index = first;
        Node * node = first;
        int maximal_y = first->data.pt->y;

        for (unsigned int ix = 0; ix < list->size(); ++ix) {
            if(node->data.pt->y > maximal_y) {
                maximal_y = node->data.pt->y;
                maximal_index = node;
            } else if(node->data.pt->y == maximal_y) {
                if(node->data.pt->x < maximal_index->data.pt->x) {
                    maximal_y = node->data.pt->y;
                    maximal_index = node;
                }
            }

            node = node->successor();
        }

        return maximal_index;
    }

    bool ear_clipping_triangulation::isDegenrate(const ear_clipping_triangulation::Node *v) {
        const Node * l = v->predecessor();
        const Node * r = v->successor();

        bool test  = sign_orientation_value(l, v, r)==0;
        return test;
    }

    bool ear_clipping_triangulation::isConvex(const ear_clipping_triangulation::Node *v,
                                            const ear_clipping_triangulation::LinkedList *list) {
        // the maximal y elemnt is always convex, therefore if
        // they have the same orientation, then v is also convex
        return neighborhood_orientation_sign(v) *
               neighborhood_orientation_sign(maximal_y_element(list)) > 0;
    }

    bool ear_clipping_triangulation::areEqual(const ear_clipping_triangulation::Node *a,
                                            const ear_clipping_triangulation::Node *b) {
        return
            a->data.pt->x==b->data.pt->x &&
            a->data.pt->y==b->data.pt->y;
    }

    bool ear_clipping_triangulation::isEmpty(const ear_clipping_triangulation::Node *v,
                                           const ear_clipping_triangulation::LinkedList *list) {
        int tsv;

        const Node * l = v->predecessor();
        const Node * r = v->successor();

        tsv = sign_orientation_value(v, l, r);

        Node * n  = list->getFirst();
        index ix = 0;

        do {

            if(areEqual(n, v) || areEqual(n, l) || areEqual(n, r))
                continue;
//            if(n==v || n==l || n==r)
//                continue;

            if(tsv * sign_orientation_value(v, l, n)>=0 &&
               tsv * sign_orientation_value(l, r, n)>=0 &&
               tsv * sign_orientation_value(r, v, n)>=0
                    )
                return false;

        } while(ix++<list->size() && (n=n->successor()));

        return true;
    }

    index ear_clipping_triangulation::required_indices_size(const index polygon_size,
                                                          const triangles::TrianglesIndices &requested) {
        switch (requested) {
            case triangles::TrianglesIndices::TRIANGLES:
            case triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY:
                return (polygon_size - 2)*3;
            default:
                return 0;
        }

    }


}
