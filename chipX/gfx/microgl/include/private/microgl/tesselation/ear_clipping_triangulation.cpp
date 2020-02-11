#include <microgl/qsort.h>

namespace microgl {
    namespace tessellation {

        template <typename number>
        bool ear_clipping_triangulation<number>::test_intersect(const vertex &a, const vertex &b,
                                                                const vertex &c, const vertex &d,
                                                                vertex & st,
                                                                bool interior_only) {
            auto ab = b - a;
            auto cd = d - c;
            auto ca = a - c;

            auto ab_cd = ab.x * cd.y - cd.x * ab.y;
//        auto s = (ab.x * ca.y - ab.y * ca.x)/ab_cd;
//        auto t = (cd.x * ca.y - cd.y * ca.x)/ab_cd;
//        bool test = interior_only ? s > 0 && s < 1 && t > 0 && t < 1 :
//                    s >= 0 && s <= 1 && t >= 0 && t <= 1;
            // as of now, I don't consider parallels as intersections
            if(abs(ab_cd)==0)
                return false;

            auto s = (ab.x * ca.y - ab.y * ca.x);
            auto t = (cd.x * ca.y - cd.y * ca.x);
            // we do these to avoid expensive divisions
            bool s_same_sign_as_abcd = (s>0 && ab_cd >0) || (s<0 && ab_cd <0);
            bool t_same_sign_as_abcd = (t>0 && ab_cd >0) || (t<0 && ab_cd <0);
            bool s_test = (s==0 || s_same_sign_as_abcd) && abs(s)<=abs(ab_cd);
            bool t_test = (t==0 || t_same_sign_as_abcd) && abs(t)<=abs(ab_cd);
//        bool s_test = (s_same_sign_as_abcd) && abs(s)<abs(ab_cd);
//        bool t_test = (t_same_sign_as_abcd) && abs(t)<abs(ab_cd);
            bool test = s_test && t_test;
            st.x = s/ab_cd;
            st.y = t/ab_cd;

            return test;
        }

        // the extremal left-bottom most vertex is always a convex vertex
        template <typename number>
        typename ear_clipping_triangulation<number>::node_t *
        ear_clipping_triangulation<number>::find_left_bottom_most_vertex(node_t * poly) {
            auto * first = poly;
            auto * candidate = first;
            vertex value = *candidate->pt;
            auto * iter = candidate->next;

            do {
                auto & v = *iter->pt;

                if(v.x < value.x) {
                    value = v;
                    candidate = iter;
                }
                else if(v.x==value.x) {
                    if(v.y > value.y) {
                        value = v;
                        candidate = iter;
                    }
                }

            } while ((iter=iter->next) && iter!=first);

            return candidate;
        }

        template <typename number>
        typename ear_clipping_triangulation<number>::node_t *
        ear_clipping_triangulation<number>::polygon_to_linked_list(vertex *$pts,
                                                                   index offset,
                                                                   index size,
                                                                   const node_type_t &type,
                                                                   bool reverse,
                                                                   pool_nodes_t &pool
        ) {
            node_t * first = nullptr, * last = nullptr;

            if (size==0)
                return nullptr;

            for (index ix = 0; ix < size; ++ix) {
                index idx = reverse ? size-1-ix : ix;
                auto * node = pool.get();
                node->pt = &$pts[idx];
                node->original_index = offset + idx;
                node->type = type;

                // record first node
                if(first== nullptr)
                    first = node;

                // build the list
                if (last) {
                    last->next = node;
                    node->prev = last;
                }

                last = node;
            }

            // make it cyclic
            last->next = first;
            first->prev = last;

            return first;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::is_bbox_overlaps_axis(const vertex &a, const vertex &b,
                                                                       const vertex &c, const vertex &d,
                                                                       bool compare_x,
                                                                       bool interior_only) {

            auto min_ab = compare_x ? a.x : a.y, max_ab = compare_x ? b.x : b.y;
            auto min_cd = compare_x ? c.x : c.y, max_cd = compare_x ? d.x : d.y;

            // sort
            if(min_ab > max_ab) {
                auto temp = min_ab;
                min_ab = max_ab;
                max_ab = temp;
            }

            if(min_cd > max_cd) {
                auto temp = min_cd;
                min_cd = max_cd;
                max_cd = temp;
            }

            bool disjoint = interior_only ? (max_cd < min_ab) && (min_cd > max_ab) :
                            (max_cd <= min_ab) && (min_cd >= max_ab);

            return !disjoint;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::is_bbox_overlaps(const vertex &a, const vertex &b,
                                                                  const vertex &c, const vertex &d,
                                                                  bool interior_only) {
            return is_bbox_overlaps_axis(a,b,c,d,true,interior_only) &&
                   is_bbox_overlaps_axis(a,b,c,d,false,interior_only);
        }


        template <typename number>
        bool ear_clipping_triangulation<number>::inCone(const vertex &a0,
                                                        const vertex &a,
                                                        const vertex &a1,
                                                        const vertex &b) {

            // is a0-a-a1 CCW
//        int sign = sign_orientation_value(&a, &a1, &a0);
            // CW for now
            int sign = sign_orientation_value(&a, &a1, &a0);

            bool is_convex = sign <= 0;
            bool in_cone;

            if(is_convex) {
                in_cone = sign_orientation_value(&a, &b, &a0)>0 && sign_orientation_value(&b, &a, &a1)>0;
            } else {
                in_cone = !(sign_orientation_value(&a, &b, &a1)>=0 && sign_orientation_value(&b, &a, &a0)>=0);
            }

            return in_cone;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::business(const node_t *v,
                                                          const vertex &a,
                                                          const vertex &b,
                                                          node_t *list) {
            int tsv;

            const node_t * l = v->next;
            const node_t * r = v->prev;

            tsv = sign_orientation_value(v->pt, l->pt, r->pt);

            // only consider CCW concave
            if(tsv==1)
                return false;

            node_t * n = list;


            auto *v_a =  n->pt;
            auto *v_b =  n->next->pt;

            bool w1 = (tsv * sign_orientation_value(v->pt, l->pt, v_a) <= 0) &&
                      (tsv * sign_orientation_value(v->pt, l->pt, v_b) <= 0);
            bool w2 = (tsv * sign_orientation_value(l->pt, r->pt, v_a) <= 0) &&
                      (tsv * sign_orientation_value(l->pt, r->pt, v_b) <= 0);
            bool w3 = (tsv * sign_orientation_value(r->pt, v->pt, v_a) <= 0) &&
                      (tsv * sign_orientation_value(r->pt, v->pt, v_b) <= 0);

            auto w4_0 = sign_orientation_value(v_a, v_b, v->pt);
            auto w4_1 = sign_orientation_value(v_a, v_b, l->pt);
            auto w4_2 = sign_orientation_value(v_a, v_b, r->pt);
            bool w4 = w4_0*w4_1>=0 && w4_0*w4_2>=0 &&  w4_1*w4_2>=0;

            bool edge_outside_triangle = w1 || w2 || w3 || w4;
            if(!edge_outside_triangle)
                return false;


//        do {
//
//            if(areEqual(n, v) || areEqual(n, l) || areEqual(n, r))
//                continue;
//
//
//            vertex m = (*n->pt + *n->next->pt)/2;
//            m = (*n->pt);
//            if(tsv * sign_orientation_value(v->pt, l->pt, &m)>0 &&
//               tsv * sign_orientation_value(l->pt, r->pt, &m)>0 &&
//               tsv * sign_orientation_value(r->pt, v->pt, &m)>0
//                    ) {
//                return false;
//            }
//
//        } while((n=n->next) && (n!=list));
//
//        return true;
        }

        template <typename number>
        typename ear_clipping_triangulation<number>::node_t *
        ear_clipping_triangulation<number>::find_mutually_visible_vertex(node_t * poly,
                                                                         const vertex& v) {

            auto * first = poly;
            auto * iter_1 = first;
            auto * iter_2 = first;
            vertex st;

            do {
//            if(iter_1->type==node_type_t::bridge)
//                continue;
                if(iter_1->pt->x > v.x)
                    continue;

                const auto &a = *(iter_1->pt);
                const auto &b = v;
                bool fails = false;
                iter_2 = first;

                do {
                    // this works because the linked-list is cyclic
                    const auto &c = *(iter_2->pt);
                    const auto &d = *(iter_2->next->pt);

                    // here we can use bbox
                    const bool bbox_overlaps = true;//is_bbox_overlaps(a,b,c,d, true);

                    if(bbox_overlaps) {
//                    bool has_mutual_endpoint = a==c || a==d || b==c || b==d;
//                    bool has_mutual_endpoint = iter_1==iter_2 || iter_1==iter_2->next;

//                    if(has_mutual_endpoint)
//                        continue;

                        bool intersects = test_intersect(a,b,c,d,st, false);

                        if(intersects) {
                            // found intersection, now let's classify and further process
                            // 1. if it's a proper intersection, we are done
                            // 2. if it's endpoints intersect in a degenerate manner, test for cone inclusion

                            bool proper = st.x>0 && st.y>0 && st.x<1 && st.y<1;

                            if(proper) {
                                fails = true;
                                break;
                            }

//                        bool intersects_at_cd_start = st.y==0;
//                        if(!intersects_at_cd_start)
//                            continue;

                            // have intersection at cd start, test if we are in a cone
                            bool in_cone = inCone(*iter_2->prev->pt, *iter_2->pt, *iter_2->next->pt, b);

                        }
                    }

                } while ((iter_2=iter_2->next) && iter_2!=first);

                if(!fails) {

//                if(iter_1->type==node_type_t::bridge) {
//                    int s_1 = sign_orientation_value(&a, &b, iter_1->prev->pt);
//                    int s_2 = sign_orientation_value(&a, &b, iter_1->next->pt);
//                    if(s_1*s_2>0)
//                        continue;
//                }

                    return iter_1;
                }

            } while ((iter_1=iter_1->next) && iter_1!=first);

            return nullptr;
        }

        template <typename number>
        void ear_clipping_triangulation<number>::merge_hole(node_t * outer,
                                                            node_t * inner,
                                                            node_t * inner_left_most_node,
                                                            pool_nodes_t &pool) {
//
            // found mutually visible vertex in outer
            auto * outer_node = find_mutually_visible_vertex(
                    outer,
                    *(inner_left_most_node->pt));

//        outer_node = outer_node->next;

            // clone the second pair of bridge
            auto * cloned_outer_node = pool.get();
            auto * cloned_inner_node = pool.get();
            cloned_inner_node->pt = inner_left_most_node->pt;
            cloned_outer_node->pt = outer_node->pt;

            cloned_inner_node->prev = inner_left_most_node->prev;
            cloned_inner_node->next = cloned_outer_node;
            inner_left_most_node->prev->next = cloned_inner_node;

            cloned_outer_node->prev = cloned_inner_node;
            cloned_outer_node->next = outer_node->next;
            outer_node->next->prev = cloned_outer_node;

            // connect outer node to inner
            outer_node->next = inner_left_most_node;
            inner_left_most_node->prev = outer_node;

            outer_node->type = node_type_t ::bridge;
            inner_left_most_node->type = node_type_t ::bridge;
            cloned_outer_node->type = node_type_t ::bridge;
            cloned_inner_node->type = node_type_t ::bridge;
        }

        template <typename number>
        int ear_clipping_triangulation<number>::compare_poly_contexts (const void * a, const void * b, void * ctx)
        {
//        auto * left_mosts = (ect::LinkedList::node_t **) ctx;
            auto * a_casted = (const poly_context_t *)a;
            auto * b_casted = (const poly_context_t *)b;

            vertex &a_lm = *(a_casted->left_most->pt);
            vertex &b_lm = *(b_casted->left_most->pt);

            bool a_before_b = a_lm.x < b_lm.x || (a_lm.x==b_lm.x && a_lm.y < b_lm.y);

            if(a_before_b)
                return -1;

            if(a_lm==b_lm)
                return 0;

            return 1;
        }

        template <typename number>
        int ear_clipping_triangulation<number>::compute(vertex *$pts,
                                                        index size,
                                                        dynamic_array<index> & indices_buffer_triangulation,
                                                        const microgl::triangles::indices &requested,
                                                        dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                                                        dynamic_array<hole> * holes,
                                                        dynamic_array<vertex > * result
        ) {

            const auto holes_count = (holes ? holes->size() : 0);
            index outer_size = size;
            for (index hx = 0; hx < holes_count; ++hx) {
                // each hole induces two more bridge points
                outer_size += (*holes)[hx].size + 2;
            }

            // this is much better to allocate memory once,
            // it will also deallocate once we we go out of scope
            pool_nodes_t pool{outer_size};

            auto * outer = polygon_to_linked_list($pts, 0, size, node_type_t::outer, false, pool);

            if(holes_count) {

                if(result== nullptr)
                    return ERR_HOLES_MUST_BE_OPERATED_WITH_RESULT_BUFFER;

                poly_context_t poly_contexts[holes_count];

                // contain holes for further processing
                for (index ix = 0; ix < holes_count; ++ix) {
                    auto hole = (*holes)[ix];
                    poly_contexts[ix].polygon = polygon_to_linked_list(hole.points, hole.offset, hole.size,
                                                                       node_type_t::inner, hole.orients_like_parent,pool);
                    poly_contexts[ix].left_most = find_left_bottom_most_vertex(poly_contexts[ix].polygon);
                    poly_contexts[ix].size = hole.size;
                }

                // sort the holes by left-most vertex
                qsort_s(poly_contexts, holes_count, sizeof(poly_context_t), compare_poly_contexts, nullptr);

                // merge holes with outer boundary
                for (index jx = 0; jx < holes_count; ++jx) {
                    merge_hole(outer,
                               poly_contexts[jx].polygon,
                               poly_contexts[jx].left_most,
                               pool);

                }

            }

            if(result) {
                auto * node = outer;
                const auto result_buffer_size = result->size();
                for (index kx = 0; kx < outer_size; ++kx) {
                    result->push_back(*node->pt);
                    node->original_index = result_buffer_size + kx;
                    node = node->next;
                }

            }

            compute(outer,
                    outer_size, indices_buffer_triangulation,
                    boundary_buffer, requested);

            return SUCCEED;
        }

        template <typename number>
        void ear_clipping_triangulation<number>::compute(vertex *polygon,
                                                         index size,
                                                         dynamic_array<index> & indices_buffer_triangulation,
                                                         dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                                                         const microgl::triangles::indices &requested) {
            compute(polygon, size, indices_buffer_triangulation, requested, boundary_buffer, nullptr, nullptr);
        }

        template <typename number>
        void ear_clipping_triangulation<number>::compute(node_t *list,
                                                         index size,
                                                         dynamic_array<index> & indices_buffer_triangulation,
                                                         dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                                                         const microgl::triangles::indices &requested) {

            bool requested_triangles_with_boundary =
                    requested==microgl::triangles::indices::TRIANGLES_WITH_BOUNDARY;
            auto &indices = indices_buffer_triangulation;

            index ind = 0;
            node_t * first = list;//->next->next->next;
            node_t * point = first;

            // remove degenerate ears, I assume, that removing all deg ears
            // will create a poly that will never have deg again (I might be wrong)
            do {
                bool is_degenerate = isDegenrate(point);

                if(!is_degenerate)
                    continue;

                point->prev->next = point->next;
                point->next->prev = point->prev;
                // set a new linked list first element
                if(point==first)
                    first=point->next;

                auto * bt = point->prev;
                // back-track backwards
                do {
                    is_degenerate = isDegenrate(bt);
                    if(!is_degenerate)
                        break;
                    bt->prev->next = bt->next;
                    bt->next->prev = bt->prev;
                } while(bt!=point->next && (bt=bt->prev));

            } while((point!=first->prev) && (point = point->next));

//        for (index ix = 0; ix < 2; ++ix) {
            for (index ix = 0; ix < size - 2; ++ix) {

                point = first;

                do {
                    bool is_degenerate = isDegenrate(point);
                    bool is_convex = isConvex(point, first);
                    bool is_empty = isEmpty(point, first);

                    if (is_degenerate || (is_convex && is_empty)) {

                        indices.push_back(point->prev->original_index);
                        indices.push_back(point->original_index);
                        indices.push_back(point->next->original_index);

                        // record boundary
                        if(requested_triangles_with_boundary) {
                            // classify if edges are on boundary
                            unsigned int first_edge_index_distance = abs((int)indices[ind + 0] - (int)indices[ind + 1]);
                            unsigned int second_edge_index_distance = abs((int)indices[ind + 1] - (int)indices[ind + 2]);
                            unsigned int third_edge_index_distance = abs((int)indices[ind + 2] - (int)indices[ind + 0]);

                            bool first_edge = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                            bool second_edge = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                            bool third_edge = third_edge_index_distance==1 || third_edge_index_distance==size-1;

                            index info = microgl::triangles::create_boundary_info(first_edge, second_edge, third_edge);

                            boundary_buffer->push_back(info);
                        }

                        ind += 3;

                        // prune the point from the polygon
                        point->prev->next = point->next;
                        point->next->prev = point->prev;
                        // set a new linked list first element
                        if(point==first)
                            first=point->next;

                        break;
                    }

                } while((point = point->next));
//            } while(point!=first->prev && (point = point->next));
//            } while((point = point->next) && point!=first);

            }

        }

        // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
        // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
        template <typename number>
        number
        ear_clipping_triangulation<number>::orientation_value(const vertex *a,
                                                              const vertex *b,
                                                              const vertex *c) {
            return (b->x - a->x)*(c->y - a->y) -
                   (c->x - a->x)*(b->y - a->y);
        }

        // ts
        template <typename number>
        int ear_clipping_triangulation<number>::neighborhood_orientation_sign(
                const node_t *v) {
            const node_t * l = v->prev;
            const node_t * r = v->next;

            // pay attention that this can return 0, although in the algorithm
            // it does not return 0 never here.
            if(sign_orientation_value(l->pt, v->pt, r->pt)==0) {
                int a=0 ;
            }


//        return sign_orientation_value(l->pt, v->pt, r->pt) > 0 ? 1 : -1;
            return sign_orientation_value(l->pt, v->pt, r->pt);// > 0 ? 1 : -1;
        }

        // tv
        template <typename number>
        char
        ear_clipping_triangulation<number>::sign_orientation_value(const vertex *i,
                                                                   const vertex *j,
                                                                   const vertex *k) {
            auto v = orientation_value(i, j, k);

            bool test  = abs(v)<0.01;
            if(test)
                return 0;

            // we clip, to avoid overflows down the road
            if(v > 0)
                return 1;
            else if(v < 0)
                return -1;
            else
                return 0;
        }

        template <typename number>
        typename ear_clipping_triangulation<number>::node_t *
        ear_clipping_triangulation<number>::maximal_y_element(node_t *list) {
            node_t * first = list;
            node_t * maximal_index = first;
            node_t * node = first;
            auto maximal_y = first->pt->y;

            do {
                if(node->pt->y > maximal_y) {
                    maximal_y = node->pt->y;
                    maximal_index = node;
                } else if(node->pt->y == maximal_y) {
                    if(node->pt->x < maximal_index->pt->x) {
                        maximal_y = node->pt->y;
                        maximal_index = node;
                    }
                }

            } while ((node = node->next) && node!=first);

            return maximal_index;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::isDegenrate(const node_t *v) {
            const node_t * l = v->prev;
            const node_t * r = v->next;

            auto val = abs(orientation_value(l->pt, v->pt, r->pt));
            bool test2  = val<0.01;
            bool test  = sign_orientation_value(l->pt, v->pt, r->pt)==0;
            return test;
//        return false;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::isConvex(const node_t *v,
                                                          node_t *list) {
//        if(neighborhood_orientation_sign(maximal_y_element(list))==0) {
//            return false;
//        }

            auto a = neighborhood_orientation_sign(v);
            auto b = neighborhood_orientation_sign(maximal_y_element(list));

            // the maximal y element is always convex, therefore if
            // they have the same orientation_t, then v is also convex
            return neighborhood_orientation_sign(v) *
                   neighborhood_orientation_sign(maximal_y_element(list)) > 0;

        }

        template <typename number>
        bool ear_clipping_triangulation<number>::areEqual(const node_t *a,
                                                          const node_t *b) {
            return a==b;
//            return a->pt->x==b->pt->x && a->pt->y==b->pt->y;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::isEmpty(const node_t *v,
                                                         node_t *list){
            int tsv;
            bool is_super_simple = false;
            const node_t * l = v->next;
            const node_t * r = v->prev;

            tsv = sign_orientation_value(v->pt, l->pt, r->pt);

            if(tsv==0)
                return true;

            node_t * n = list;

            do {

                if(areEqual(n, v) || areEqual(n, l) || areEqual(n, r))
                    continue;

                if(is_super_simple) {
                    vertex m = (*n->pt);
                    if(tsv * sign_orientation_value(v->pt, l->pt, &m)>0 &&
                       tsv * sign_orientation_value(l->pt, r->pt, &m)>0 &&
                       tsv * sign_orientation_value(r->pt, v->pt, &m)>0
                            ) {
                        return false;
                    }

                } else {
                    // this can handle small degenerate cases, we basically test
                    // if the interior is completely empty, if we have used the regular
                    // tests than the degenerate cases where things just touch would fail the test
                    auto *v_a =  n->pt;
                    auto *v_b =  n->next->pt;

                    // todo:: can break prematurely instead of calcing everything
                    bool w1 = (tsv * sign_orientation_value(v->pt, l->pt, v_a) <= 0) &&
                              (tsv * sign_orientation_value(v->pt, l->pt, v_b) <= 0);
                    bool w2 = (tsv * sign_orientation_value(l->pt, r->pt, v_a) <= 0) &&
                              (tsv * sign_orientation_value(l->pt, r->pt, v_b) <= 0);
                    bool w3 = (tsv * sign_orientation_value(r->pt, v->pt, v_a) <= 0) &&
                              (tsv * sign_orientation_value(r->pt, v->pt, v_b) <= 0);

                    auto w4_0 = sign_orientation_value(v_a, v_b, v->pt);
                    auto w4_1 = sign_orientation_value(v_a, v_b, l->pt);
                    auto w4_2 = sign_orientation_value(v_a, v_b, r->pt);
                    bool w4 = w4_0*w4_1>=0 && w4_0*w4_2>=0 &&  w4_1*w4_2>=0;

                    bool edge_outside_triangle = w1 || w2 || w3 || w4;
                    if(!edge_outside_triangle)
                        return false;
                }


            } while((n=n->next) && (n!=list));

            return true;
        }

        template <typename number>
        index ear_clipping_triangulation<number>::required_indices_size(const index polygon_size,
                                                                        const microgl::triangles::indices &requested) {
            switch (requested) {
                case microgl::triangles::indices::TRIANGLES:
                case microgl::triangles::indices::TRIANGLES_WITH_BOUNDARY:
                    return (polygon_size - 2)*3;
                default:
                    return 0;
            }

        }


    }

}
