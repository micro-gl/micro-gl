namespace microgl {
    namespace tessellation {

        template<typename number, template<typename...> class container_type>
        auto
        monotone_polygon_triangulation<number, container_type>::polygon_to_linked_list(const vertex *$pts,
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
            for (int ix = 0; ix < 2; ++ix) {
                if(isDegenerate(last)){
                    last->prev->next=last->next;
                    last->next->prev=last->prev;
                    auto *new_last=last->prev;
                    last->prev=last->next= nullptr;
                    last=new_last;
                }
            }
            return last;
        }

        template<typename number, template<typename...> class container_type>
        void monotone_polygon_triangulation<number, container_type>::compute(const vertex *polygon,
                                                         index size,
                                                         const monotone_axis & axis,
                                                         container_type<index> & indices_buffer_triangulation,
                                                         container_type<microgl::triangles::boundary_info> * boundary_buffer,
                                                         microgl::triangles::indices &output_type) {
            if(size<=2) return;
            pool_nodes_t pool{size};
            auto * outer = polygon_to_linked_list(polygon, 0, size, false, pool);
            compute(outer, size, axis, indices_buffer_triangulation, boundary_buffer, output_type);
        }

        template<typename number, template<typename...> class container_type>
        void monotone_polygon_triangulation<number, container_type>::triangle(
                container_type<index> & indices_buffer_triangulation,
                container_type<microgl::triangles::boundary_info> * boundary_buffer,
                int v0_index, int v1_index, int v2_index, int size) {
            indices_buffer_triangulation.push_back(v0_index);
            indices_buffer_triangulation.push_back(v1_index);
            indices_buffer_triangulation.push_back(v2_index);
            if(boundary_buffer) {
#define abs_monotone(a) ((a)<0 ? -(a) : (a))
                int first_edge_index_distance = abs_monotone(v0_index - v1_index);
                int second_edge_index_distance = abs_monotone(v1_index- v2_index);
                int third_edge_index_distance = abs_monotone(v2_index - v0_index);
#undef abs_monotone
                bool first_edge = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                bool second_edge = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                bool third_edge = third_edge_index_distance==1 || third_edge_index_distance==size-1;
                boundary_buffer->push_back(triangles::create_boundary_info(first_edge, second_edge, third_edge));
            }
        }

        template<typename number, template<typename...> class container_type>
        void monotone_polygon_triangulation<number, container_type>::compute(node_t *list,
                                                             index size,
                                                             const monotone_axis & axis,
                                                             container_type<index> & indices_buffer_triangulation,
                                                             container_type<microgl::triangles::boundary_info> * boundary_buffer,
                                                             microgl::triangles::indices &output_type) {
            bool requested_triangles_with_boundary = boundary_buffer;
            output_type=requested_triangles_with_boundary? microgl::triangles::indices::TRIANGLES_WITH_BOUNDARY :
                        microgl::triangles::indices::TRIANGLES;
            auto &indices = indices_buffer_triangulation;
            // find monotone
            node_t *min, *max, *iter;
            find_min_max(list, axis, &min, &max);
            int poly_orientation_sign=classify_point(*min->prev->pt, *min->pt, *min->next->pt);
            orientation poly_orientation=poly_orientation_sign==-1?orientation::cw :orientation::ccw;
            const bool is_poly_cw= poly_orientation==orientation::cw;
            if(poly_orientation_sign==0) return;

            iter=min;
            while (iter!=max) { // classify chain B as top chain from_sampler min(including) up to max(not including)
                iter->chain_index=1;
                iter=is_poly_cw?iter->next:iter->prev;
            }
            // merge sort chains into an increasing sequence, chain A is the lowest, while B is on top
            node_t *iter_chain_a=min, *iter_chain_b=is_poly_cw?min->next:min->prev;
            auto ** sorted_list= new node_t*[size];
            index count=0;
            while(count<size) {
                if(a_B_b(iter_chain_a, iter_chain_b, axis)) {
                    sorted_list[count]=iter_chain_a;
                    iter_chain_a=is_poly_cw?iter_chain_a->prev:iter_chain_a->next;
                } else {
                    sorted_list[count]=iter_chain_b;
                    iter_chain_b=is_poly_cw?iter_chain_b->next:iter_chain_b->prev;
                }
                count++;
            }
            // start
            dynamic_array<node_t *> stack{16};
            stack.push_back(sorted_list[0]);
            stack.push_back(sorted_list[1]);
            for (unsigned j = 2; j < size - 1; ++j) {
                auto * u_j= sorted_list[j];
                auto * stack_top=stack.back();
                bool on_different_chains= u_j->chain_index != stack_top->chain_index;
                if(on_different_chains) {
                    if(stack.size()>=2) {
                        // insert a diagonal to every point on the stack except the last one
                        for (unsigned ix = 0; ix < stack.size()-1; ++ix) {
                            const node_t * a=stack[ix];
                            const node_t * b=stack[ix+1];
                            triangle(indices, boundary_buffer, u_j->original_index,
                                    a->original_index, b->original_index, size);
                        }
                        stack.clear(); // empty the stack
                        // push u_(j-1) and u_j
                        stack.push_back(sorted_list[j-1]);
                        stack.push_back(u_j);
                    }
                } else {
                    // search for the longest chain for which diagonals work
                    bool is_top_chain=u_j->chain_index==1;
                    if(stack.size()>=2) {
                        unsigned s_index=stack.size()-1;
                        for (unsigned ix = s_index; ix >= 1; --ix) {
                            const node_t * a=stack[ix-1];
                            const node_t * b=stack[ix];
                            int cls=classify_point(*u_j->pt, *a->pt, *b->pt);
                            bool is_inside=is_top_chain?cls<0 : cls>0;
                            if(is_inside) {
                                triangle(indices, boundary_buffer, u_j->original_index,
                                         a->original_index, b->original_index, size);
                                stack.pop_back();
                            } else break;
                        }
                    }
                    stack.push_back(u_j);
                }
            }
            // add diagonals from_sampler u_n to the rest of points remaining on the stack
            auto * u_n= sorted_list[size-1];
            if(stack.size()>=2) {
                for (unsigned ix = 0; ix < stack.size()-1; ++ix) {
                    const node_t * a=stack[ix];
                    const node_t * b=stack[ix+1];
                    triangle(indices, boundary_buffer, u_n->original_index,
                             a->original_index, b->original_index, size);
                }
            }
            delete [] sorted_list;
        }

        template<typename number, template<typename...> class container_type>
        bool monotone_polygon_triangulation<number, container_type>::a_B_b(node_t *a, node_t *b, const monotone_axis & axis) {
            bool is_x_monotone= axis==monotone_axis::x_monotone;
            bool is_before=is_x_monotone ? (a->pt->x<b->pt->x || (a->pt->x==b->pt->x && a->pt->y<b->pt->y)) :
                           (a->pt->y<b->pt->y || (a->pt->y==b->pt->y && a->pt->x<b->pt->x));
            return is_before;
        }

        template<typename number, template<typename...> class container_type>
        bool monotone_polygon_triangulation<number, container_type>::a_G_b(node_t *a, node_t *b, const monotone_axis & axis) {
            bool is_x_monotone= axis==monotone_axis::x_monotone;
            bool is_before=is_x_monotone ? (a->pt->x>b->pt->x || (a->pt->x==b->pt->x && a->pt->y>b->pt->y)) :
                           (a->pt->y>b->pt->y || (a->pt->y==b->pt->y && a->pt->x>b->pt->x));
            return is_before;
        }

        template<typename number, template<typename...> class container_type>
        inline int
        monotone_polygon_triangulation<number, container_type>::classify_point(const vertex & point, const vertex &a, const vertex & b) {
            // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
            // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
            //    Input:  three points p, a, b
            //    Return: >0 for P left of the line through a and b
            //            =0 for P  on the line
            //            <0 for P  right of the line
            //    See: Algorithm 1 "Area of Triangles and Polygons"
            auto result= (b.x-a.x)*(point.y-a.y)-(point.x-a.x)*(b.y-a.y);
            if(result<0) return 1;
            else if(result>0) return -1;
            else return 0;
        }

        template<typename number, template<typename...> class container_type>
        bool monotone_polygon_triangulation<number, container_type>::isDegenerate(const node_t *v) {
            return classify_point(*v->prev->pt, *v->pt, *v->next->pt)==0;
        }

        template<typename number, template<typename...> class container_type>
        auto monotone_polygon_triangulation<number, container_type>::find_min_max(node_t *list, const monotone_axis &axis,
                                                                  node_t **min, node_t **max) -> void {
            *min = list, *max=list;
            node_t * iter = list;
            do {
                bool is_new_min=a_B_b(iter, *min, axis);
                bool is_new_max=a_G_b(iter, *max, axis);
                if(is_new_min) *min=iter;
                if(is_new_max) *max=iter;
            } while ((iter = iter->next) && iter != list);

        }

    }

}
