namespace microgl {
    namespace tessellation {

        template <typename number>
        auto
        ear_clipping_triangulation<number>::polygon_to_linked_list(const vertex *$pts,
                                                                   index offset,
                                                                   index size,
                                                                   bool reverse,
                                                                   pool_nodes_t &pool) -> node_t * {
            node_t * first = nullptr, * last = nullptr;
            if (size<=2) return nullptr;
            for (index ix = 0; ix < size; ++ix) {
                index idx = reverse ? size-1-ix : ix;
                auto * node = pool.get();
                node->pt = &$pts[idx];
                node->original_index = offset + idx;
                // record first node
                if(first== nullptr) first = node;
                // build the list
                if (last) {
                    last->next = node;
                    node->prev = last;
                }
                last = node;
                auto * candidate_deg=last->prev;
                if(ix>=2 && isDegenerate(candidate_deg)){
                    candidate_deg->prev->next=candidate_deg->next;
                    candidate_deg->next->prev=candidate_deg->prev;
                    candidate_deg->prev=candidate_deg->next= nullptr;
                }
            }
            // make it cyclic
            last->next = first;
            first->prev = last;
            if(isDegenerate(last)){
                last->prev->next=last->next;
                last->next->prev=last->prev;
                last->prev=last->next= nullptr;
            }
            return first;
        }

        template <typename number>
        void ear_clipping_triangulation<number>::compute(const vertex *polygon,
                                                         index size,
                                                         dynamic_array<index> & indices_buffer_triangulation,
                                                         dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                                                         const microgl::triangles::indices &requested) {
            pool_nodes_t pool{size};
            auto * outer = polygon_to_linked_list(polygon, 0, size, false, pool);
            compute(outer, size, indices_buffer_triangulation, boundary_buffer, requested);
        }

        template <typename number>
        void ear_clipping_triangulation<number>::update_ear_status(node_t *vertex, const int &polygon_orientation) {
            if(!vertex->isValid()) {
                vertex->is_ear=false;
                return;
            }
            int vertex_orient=neighborhood_orientation_sign(vertex);
            bool is_convex = vertex_orient*polygon_orientation>0; // same orientation as polygon
            bool is_empty = isEmpty(vertex);
            vertex->is_ear=is_convex && is_empty;
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
            node_t * first = list;
            node_t * point = first;
            int poly_orient=neighborhood_orientation_sign(maximal_y_element(first));
            if(poly_orient==0) return;
            do {
                update_ear_status(point, poly_orient);
            } while((point=point->next) && point!=first);
            // remove degenerate ears, I assume, that removing all deg ears
            // will create a poly that will never have deg again (I might be wrong)
            for (index ix = 0; ix < size - 2; ++ix) {
                point = first;
                if(point== nullptr) break;
                do {
                    bool is_ear=point->is_ear;
                    if (is_ear) {
                        indices.push_back(point->prev->original_index);
                        indices.push_back(point->original_index);
                        indices.push_back(point->next->original_index);
                        // record boundary
                        if(requested_triangles_with_boundary) {
                            // classify if edges are on boundary
                            unsigned int first_edge_index_distance = abs_((int)indices[ind + 0] - (int)indices[ind + 1]);
                            unsigned int second_edge_index_distance = abs_((int)indices[ind + 1] - (int)indices[ind + 2]);
                            unsigned int third_edge_index_distance = abs_((int)indices[ind + 2] - (int)indices[ind + 0]);
                            bool first_edge = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                            bool second_edge = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                            bool third_edge = third_edge_index_distance==1 || third_edge_index_distance==size-1;
                            index info = microgl::triangles::create_boundary_info(first_edge, second_edge, third_edge);
                            boundary_buffer->push_back(info);
                            ind += 3;
                        }
                        // prune the point from the polygon
                        point->prev->next = point->next;
                        point->next->prev = point->prev;
                        auto* anchor_prev=point->prev, * anchor_next=point->next;
                        point->prev=point->next= nullptr;
                        anchor_prev=remove_degenerate_from(anchor_prev, true);
                        anchor_next=remove_degenerate_from(anchor_next, false);
                        update_ear_status(anchor_prev, poly_orient);
                        update_ear_status(anchor_next, poly_orient);
                        if(anchor_prev && anchor_prev->isValid()) first=anchor_prev;
                        else if(anchor_next && anchor_next->isValid()) first=anchor_next;
                        else first= nullptr;
                        break;
                    }
                } while((point = point->next) && point!=first);
            }
        }

        template <typename number>
        auto ear_clipping_triangulation<number>::remove_degenerate_from(node_t *v, bool backwards) ->node_t * {
            if(!v->isValid()) return v;
            node_t* anchor=v;
            while (anchor->isValid() && isDegenerate(anchor)) {
                auto * prev=anchor->prev, * next=anchor->next;
                // rewire
                prev->next = next;
                next->prev = prev;
                anchor->prev=anchor->next= nullptr;
                anchor=backwards ? prev : next;
            }
            return anchor;
        }

        template <typename number>
        number
        ear_clipping_triangulation<number>::orientation_value(const vertex *a,
                                                              const vertex *b,
                                                              const vertex *c) {
            // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
            // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
            return (b->x-a->x)*(c->y-a->y) - (c->x-a->x)*(b->y-a->y);
        }

        // ts
        template <typename number>
        int ear_clipping_triangulation<number>::neighborhood_orientation_sign(
                const node_t *v) {
            return sign_orientation_value(v->prev->pt, v->pt, v->next->pt);
        }

        // tv
        template <typename number>
        char
        ear_clipping_triangulation<number>::sign_orientation_value(const vertex *i,
                                                                   const vertex *j,
                                                                   const vertex *k) {
            auto v = orientation_value(i, j, k);
//            bool test  = abs(v)<1;
//            if(test) return 0;
            if(v > 0) return 1;
            else if(v < 0) return -1;
            else return 0;
        }

        template <typename number>
        auto ear_clipping_triangulation<number>::maximal_y_element(node_t *list) -> node_t * {
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
        bool ear_clipping_triangulation<number>::isDegenerate(const node_t *v) {
            return sign_orientation_value(v->prev->pt, v->pt, v->next->pt)==0;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::areEqual(const node_t *a,
                                                          const node_t *b) {
            return a==b;
        }

        template <typename number>
        bool ear_clipping_triangulation<number>::isEmpty(node_t *v) {
            int tsv;
            bool is_super_simple = false;
            const node_t * l = v->next;
            const node_t * r = v->prev;
            tsv = sign_orientation_value(v->pt, l->pt, r->pt);
            if(tsv==0) return true;
            node_t * n = v;
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
                    if(!edge_outside_triangle) return false;
                }
            } while((n=n->next) && (n!=v));
            return true;
        }

    }

}
