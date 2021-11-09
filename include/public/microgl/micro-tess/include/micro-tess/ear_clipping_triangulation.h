/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "vec2.h"
#include "triangles.h"
#include "std_rebind_allocator.h"

namespace microtess {

    using index = unsigned int;

    /**
     * Ear Clipping Tesselation for simple polygons
     *
     * 1. we make this O(n^2) by pre computing if a vertex is an ear, and every time we remove
     *    an ear, only recompute for it's two adjacent vertices.
     * 2. This algorithm was customized by me tolerate simple polygons with touching vertices on edges,
     *    which is important. It can also tolerate holes which is cool, BUT not opposing edges on edges
     *    in order to create perfect hole (this will not work due to aliasing).
     * 3. I also remove degenerate vertices as I go to make it easier on the algorithm.
     * 4. I can also make the sign function more robust but it really won't matter much.
     * 5. I can also make it an O(r*n) algorithm where r are the number of reflex(concave) vertices,
     *    simply track them, and when testing for earness of a vertex, compare it against
     *    these reflex vertices.
     * 6. The strength of this algorithm is it's simplicity, short code, stability, low memory usage,
     *    does not require crazy numeric robustness.
     *
     * @tparam number the number type of a vertex
     * @tparam container_output_indices output indices container type
     * @tparam container_output_boundary output boundary info container type
     * @tparam computation_allocator allocator for internal computation
     */
    template<typename number,
             class container_output_indices,
             class container_output_boundary,
             class computation_allocator=microtess::std_rebind_allocator<>>
    class ear_clipping_triangulation {
    public:
        using vertex = microtess::vec2<number>;

        struct node_t {
        private:
            // this is the linked-list data
            // this is a compund data structure:
            // [ 2*origin_index | 1 bit for if this is an ear]
            // part A is always even, therefore the last bit is unused, so we
            // use it for ear status.
            index _original_index_and_ear_status = 0;
            static const index MASK = index(1);
            static const index MASK_REVERSED = ~MASK;

        public:
            node_t() = default;
            node_t *prev = nullptr;
            node_t *next = nullptr;

            void set_original_index(index original_index_of_point) {
                // warning, this resets the chain status flag
                _original_index_and_ear_status = original_index_of_point*2;
            }
            index original_index() const {
                return index(_original_index_and_ear_status & MASK_REVERSED) / 2;
            }

            void set_is_ear(bool is) {
                if(is) _original_index_and_ear_status |= MASK;
                else _original_index_and_ear_status &= MASK_REVERSED;
            }

            bool is_ear() const { return _original_index_and_ear_status & MASK; }
            bool isValid() { return prev && next;}
        };

    private:
        // rebinded allocator for nodes
        using rebind_alloc = typename computation_allocator::template rebind<node_t>::other;

        struct pool_nodes_t {

            explicit pool_nodes_t(const index count,
                                  const computation_allocator & copy_alloc) :
                                  _allocator{copy_alloc}, _count{count} {
                pool = _allocator.allocate(count);
                for (int ix = 0; ix < count; ++ix)
                    new (pool + ix) node_t();
            }

            ~pool_nodes_t() { _allocator.deallocate(pool, _count); }
            node_t *get() { return (pool + _current++); }

        private:
            rebind_alloc _allocator;
            node_t *pool = nullptr;
            index _current = 0;
            index _count = 0;
        };

    public:
        ear_clipping_triangulation()=delete;
        ear_clipping_triangulation(const ear_clipping_triangulation &)=delete;
        ear_clipping_triangulation(ear_clipping_triangulation &&)=delete;
        ear_clipping_triangulation & operator=(const ear_clipping_triangulation &)=delete;
        ear_clipping_triangulation & operator=(ear_clipping_triangulation &&)=delete;
        ~ear_clipping_triangulation()=delete;

        static void compute(const vertex *polygon,
                            index size,
                            container_output_indices &indices_buffer_triangulation,
                            container_output_boundary *boundary_buffer,
                            microtess::triangles::indices &output_type,
                            const computation_allocator & allocator=computation_allocator()) {
            pool_nodes_t pool{size, allocator};
            auto * outer = polygon_to_linked_list(polygon, 0, size, false, pool);
            compute(polygon, outer, size, indices_buffer_triangulation, boundary_buffer, output_type);
        }

    private:

        static
        node_t *polygon_to_linked_list(const vertex *$pts,
                                       index offset,
                                       index size,
                                       bool reverse,
                                       pool_nodes_t & pool) {
            node_t * first = nullptr, * last = nullptr;
            if (size<=2) return nullptr;
            for (index ix = 0; ix < size; ++ix) {
                index idx = reverse ? size-1-ix : ix;
                auto * node = pool.get();
                node->set_original_index(offset + idx);
                // record first node
                if(first== nullptr) first = node;
                // build the list
                if (last) {
                    last->next = node;
                    node->prev = last;
                }
                last = node;
                auto * candidate_deg=last->prev;
                if(ix>=2 && isDegenerate(candidate_deg, $pts)){
                    candidate_deg->prev->next=candidate_deg->next;
                    candidate_deg->next->prev=candidate_deg->prev;
                    candidate_deg->prev=candidate_deg->next= nullptr;
                }
            }
            // make it cyclic
            last->next = first;
            first->prev = last;
            for (int ix = 0; ix < 2; ++ix) {
                if(isDegenerate(last, $pts)){
                    last->prev->next=last->next;
                    last->next->prev=last->prev;
                    auto *new_last=last->prev;
                    last->prev=last->next= nullptr;
                    last=new_last;
                }
            }
            return last;
        }

        static void compute(const vertex *polygon,
                            node_t *list,
                            index size,
                            container_output_indices &indices_buffer_triangulation,
                            container_output_boundary *boundary_buffer,
                            microtess::triangles::indices &output_type) {
            bool requested_triangles_with_boundary = boundary_buffer;
            output_type=requested_triangles_with_boundary? microtess::triangles::indices::TRIANGLES_WITH_BOUNDARY :
                        microtess::triangles::indices::TRIANGLES;
            auto &indices = indices_buffer_triangulation;
            index ind = 0;
            node_t * first = list;
            node_t * point = first;
            int poly_orient=neighborhood_orientation_sign(maximal_y_element(first, polygon), polygon);
            if(poly_orient==0) return;
            do {
                update_ear_status(point, polygon, poly_orient);
            } while((point=point->next) && point!=first);
            // remove degenerate ears, I assume, that removing all deg ears
            // will create a poly that will never have deg again (I might be wrong)
            for (index ix = 0; ix < size - 2; ++ix) {
                point = first;
                if(point== nullptr) break;
                do {
                    bool is_ear=point->is_ear();
                    if (is_ear) {
                        indices.push_back(point->prev->original_index());
                        indices.push_back(point->original_index());
                        indices.push_back(point->next->original_index());
                        // record boundary
                        if(requested_triangles_with_boundary) {
                            // classify if edges are on boundary
#define abs_ear(a) ((a)<0 ? -(a) : (a))
                            unsigned int first_edge_index_distance = abs_ear((int)indices[ind + 0] - (int)indices[ind + 1]);
                            unsigned int second_edge_index_distance = abs_ear((int)indices[ind + 1] - (int)indices[ind + 2]);
                            unsigned int third_edge_index_distance = abs_ear((int)indices[ind + 2] - (int)indices[ind + 0]);
#undef abs_ear
                            bool first_edge = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                            bool second_edge = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                            bool third_edge = third_edge_index_distance==1 || third_edge_index_distance==size-1;
                            index info = microtess::triangles::create_boundary_info(first_edge, second_edge, third_edge);
                            boundary_buffer->push_back(info);
                            ind += 3;
                        }
                        // prune the point from the polygon
                        point->prev->next = point->next;
                        point->next->prev = point->prev;
                        auto* anchor_prev=point->prev, * anchor_next=point->next;
                        point->prev=point->next= nullptr;
                        anchor_prev=remove_degenerate_from(anchor_prev, polygon, true);
                        anchor_next=remove_degenerate_from(anchor_next, polygon, false);
                        update_ear_status(anchor_prev, polygon, poly_orient);
                        update_ear_status(anchor_next, polygon, poly_orient);
                        if(anchor_prev && anchor_prev->isValid()) first=anchor_prev;
                        else if(anchor_next && anchor_next->isValid()) first=anchor_next;
                        else first= nullptr;
                        break;
                    }
                } while((point = point->next) && point!=first);
            }
        }

        static number orientation_value(const vertex &a, const vertex &b, const vertex &c) {
            // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
            // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
            return (b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y);
        }

        static int neighborhood_orientation_sign(const node_t *v, const vertex *polygon) {
            return sign_orientation_value(polygon[v->prev->original_index()],
                                          polygon[v->original_index()],
                                          polygon[v->next->original_index()]);
        }

        static char sign_orientation_value(const vertex &i, const vertex &j, const vertex &k) {
            auto v = orientation_value(i, j, k);
            if(v > 0) return 1;
            else if(v < 0) return -1;
            else return 0;
        }

        static node_t *maximal_y_element(node_t *list, const vertex * polygon) {
            auto get_point = [polygon] (const node_t * node) -> const vertex & {
                return polygon[node->original_index()];
            };

            node_t * first = list;
            node_t * maximal_index = first;
            node_t * node = first;
            auto maximal_y = get_point(first).y;
            do {
                if(get_point(node).y > maximal_y) {
                    maximal_y = get_point(node).y;
                    maximal_index = node;
                } else if(get_point(node).y == maximal_y) {
                    if(get_point(node).x < get_point(maximal_index).x) {
                        maximal_y = get_point(node).y;
                        maximal_index = node;
                    }
                }
            } while ((node = node->next) && node!=first);
            return maximal_index;
        }

        static bool isEmpty(node_t *v, const vertex * polygon) {
            const auto get_point = [polygon] (const node_t * node) -> const vertex & {
                return polygon[node->original_index()];
            };

            int tsv;
            bool is_super_simple = false;
            const node_t * l = v->next;
            const node_t * r = v->prev;
            tsv = sign_orientation_value(get_point(v), get_point(l), get_point(r));
            if(tsv==0) return true;
            node_t * n = v;
            do {
                if(areEqual(n, v) || areEqual(n, l) || areEqual(n, r))
                    continue;
                if(is_super_simple) {
                    vertex m = get_point(n);
                    if(tsv * sign_orientation_value(get_point(v), get_point(l), m)>0 &&
                       tsv * sign_orientation_value(get_point(l), get_point(r), m)>0 &&
                       tsv * sign_orientation_value(get_point(r), get_point(v), m)>0) {
                        return false;
                    }
                } else {
                    // this can handle small degenerate cases, we basically test_texture
                    // if the interior is completely empty, if we have used the regular
                    // tests than the degenerate cases where things just touch would fail the test_texture
                    auto & v_a =  get_point(n);
                    auto & v_b =  get_point(n->next);
                    // todo:: can break prematurely instead of calcing everything
                    bool w1 = (tsv * sign_orientation_value(get_point(v), get_point(l), v_a) <= 0) &&
                              (tsv * sign_orientation_value(get_point(v), get_point(l), v_b) <= 0);
                    bool w2 = (tsv * sign_orientation_value(get_point(l), get_point(r), v_a) <= 0) &&
                              (tsv * sign_orientation_value(get_point(l), get_point(r), v_b) <= 0);
                    bool w3 = (tsv * sign_orientation_value(get_point(r), get_point(v), v_a) <= 0) &&
                              (tsv * sign_orientation_value(get_point(r), get_point(v), v_b) <= 0);
                    auto w4_0 = sign_orientation_value(v_a, v_b, get_point(v));
                    auto w4_1 = sign_orientation_value(v_a, v_b, get_point(l));
                    auto w4_2 = sign_orientation_value(v_a, v_b, get_point(r));
                    bool w4 = w4_0*w4_1>=0 && w4_0*w4_2>=0 &&  w4_1*w4_2>=0;
                    bool edge_outside_triangle = w1 || w2 || w3 || w4;
                    if(!edge_outside_triangle) return false;
                }
            } while((n=n->next) && (n!=v));
            return true;
        }

        static bool areEqual(const node_t *a, const node_t *b) {
            return a==b;
        }

        static bool isDegenerate(const node_t *v, const vertex * polygon) {
            return sign_orientation_value(polygon[v->prev->original_index()],
                                          polygon[v->original_index()],
                                          polygon[v->next->original_index()])==0;
        }

        static auto remove_degenerate_from(node_t *v, const vertex * polygon, bool backwards) -> node_t * {
            if(!v->isValid()) return v;
            node_t* anchor=v;
            while (anchor->isValid() && isDegenerate(anchor, polygon)) {
                auto * prev=anchor->prev, * next=anchor->next;
                // rewire
                prev->next = next;
                next->prev = prev;
                anchor->prev=anchor->next= nullptr;
                anchor=backwards ? prev : next;
            }
            return anchor;
        }

        static void update_ear_status(node_t *node, const vertex * polygon, const int &polygon_orientation) {
            if(!node->isValid()) {
                node->set_is_ear(false);
                return;
            }
            int vertex_orient=neighborhood_orientation_sign(node, polygon);
            bool is_convex = vertex_orient*polygon_orientation>0; // same orientation as polygon
            bool is_empty = isEmpty(node, polygon);
            node->set_is_ear(is_convex && is_empty);
        }

    };

}
