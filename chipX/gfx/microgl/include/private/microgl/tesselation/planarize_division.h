#pragma once
#define DEBUG_PLANAR true
#define MAX_ITERATIONS 200
#define APPLY_MERGE false

#include <microgl/tesselation/half_edge.h>
#include <microgl/chunker.h>

#if DEBUG_PLANAR==true
#include <stdexcept>
#include <string>
#define string_debug(msg) std::string(msg)
#define throw_debug(msg, poly, edge, count) throw std::runtime_error(msg+" | poly #" + std::to_string(poly) \
                                        +"| edge # "+std::to_string(edge) \
                                         +"| count # "+std::to_string(count));
#define throw_regular(msg) throw std::runtime_error(msg);
#else
#define throw_debug(msg, poly, edge, count);
#define throw_regular(msg);
#define string_debug(msg);
#endif

namespace microgl {

    namespace tessellation {
#define abs_(a) ((a)<0 ? -(a) : (a))

        enum class fill_rule {
            non_zero, even_odd
        };

        enum class tess_quality {
            // the fastest algorithm, but nay produce zero area triangles on the
            // boundary. might be a problem if you are using SDF based AA
            fine,
            // a bit slower and might be susceptible for other issues, but produces
            // triangles out of each trapeze in a way similar to ear clipping, this
            // fights zero area triangles on the boundary, so you can use SDF AA
            better,
            // fast algorithm, that produces eye pleasing reaults, but uses around
            // x2 memory for indices because it adds a center vertex in a trapeze and
            // therefore adds two more triangles per trapeze on average
            prettier_with_extra_vertices
        };

        template <typename number>
        class planarize_division {
        private:
#define min__(a, b) ((a)<(b) ? (a) : (b))
#define max__(a, b) ((a)>(b) ? (a) : (b))
            using vertex = microgl::vec2<number>;
            using index = unsigned int;
            using half_edge = half_edge_t<number>;
            using half_edge_vertex = half_edge_vertex_t<number>;
            using half_edge_face = half_edge_face_t<number>;
            using conflict = conflict_node_t<number>;
            using poly_info = poly_info_t<number>;

            struct dynamic_pool {
            private:
                index t = 1;
                dynamic_array<half_edge_vertex *> _vertices;
                dynamic_array<half_edge *> _edges;
                dynamic_array<half_edge_face *> _faces;

            public:
                ~dynamic_pool() {
                    const auto size_v = _vertices.size();
                    const auto size_e = _edges.size();
                    const auto size_f = _faces.size();
                    for (index ix = 0; ix < size_v; ++ix)
                        delete _vertices.data()[ix];
                    for (index ix = 0; ix < size_e; ++ix)
                        delete _edges.data()[ix];
                    for (index ix = 0; ix < size_f; ++ix)
                        delete _faces.data()[ix];
                };

                auto create_vertex(const vertex &coords) -> half_edge_vertex * {
                    auto * v = new half_edge_vertex();
                    v->coords = coords;
//                    v->head_id=
                    v->id=_vertices.push_back(v);
                    if(v->id==43) {
                        int debug=1;
                    }
                    return v;
                }

                auto create_edge() -> half_edge * {
                    auto * v = new half_edge();
                    _edges.push_back(v);
                    return v;
                }

                auto create_face() -> half_edge_face * {
                    auto * v = new half_edge_face();
                    v->index=t++;
                    _faces.push_back(v);
                    return v;
                }

                auto getFaces() -> dynamic_array<half_edge_face *> & {
                    return _faces;
                }

            };

            struct trapeze_t {
                // in ccw order
                half_edge *left_top=nullptr;
                half_edge *left_bottom=nullptr;
                half_edge *right_bottom=nullptr;
                half_edge *right_top=nullptr;
                bool isSimple() {
                    return left_top->next==left_bottom && left_bottom->next==right_bottom &&
                           right_bottom->next==right_top && right_top->next==left_top;
                }
                bool isDeg() const {
                    return (left_top->origin->coords==left_bottom->origin->coords &&
                            right_top->origin->coords==right_bottom->origin->coords) ||
                            (left_top->origin->coords==right_top->origin->coords &&
                             left_bottom->origin->coords==right_bottom->origin->coords);
                }
            };

            enum class point_class_with_trapeze {
                // all of the walls enums are actually strictly inside excluding endpoints, open segments
                strictly_inside, left_wall, right_wall, top_wall, bottom_wall, boundary_vertex, outside, unknown
            };

            enum class intersection_status {
                intersect, none, parallel, degenerate_line
            };

            struct conflicting_edge_intersection_status {
                vertex point_of_interest;
                point_class_with_trapeze class_of_interest_point;
            };

            struct vertical_face_cut_result {
                trapeze_t left_trapeze;
                trapeze_t right_trapeze;
                half_edge_vertex * vertex = nullptr;
                half_edge * vertex_a_edge_split_edge = nullptr;
                // true/false if was split into two
                bool face_was_split = false;
            };

            struct face_split_result {
                half_edge_vertex *planar_vertex_a= nullptr, *planar_vertex_b=nullptr;
                bool has_horizontal_split=false;
            };

            struct vertex_location_result {
                point_class_with_trapeze classs=point_class_with_trapeze::unknown;
                half_edge *outgoing_edge= nullptr;
                vertex_location_result(point_class_with_trapeze $classs, half_edge *$outgoing_edge)
                : classs{$classs}, outgoing_edge{$outgoing_edge}{

                }
            };

            struct location_codes {
                int left_wall, bottom_wall, right_wall, top_wall;
                int array[4];
                void fill() {
                    array[indexOfClass(point_class_with_trapeze::left_wall)]=left_wall;
                    array[indexOfClass(point_class_with_trapeze::bottom_wall)]=bottom_wall;
                    array[indexOfClass(point_class_with_trapeze::right_wall)]=right_wall;
                    array[indexOfClass(point_class_with_trapeze::top_wall)]=top_wall;
                }
                void compute_location_codes(const vertex & point, const trapeze_t &trapeze) {
                    // reminder: 0=on wall, -1=right of wall, 1= left of wall
                    const number pre_left_wall = point.x-trapeze.left_top->origin->coords.x;
                    const number pre_right_wall = -point.x+trapeze.right_top->origin->coords.x;
                    left_wall= pre_left_wall<number(0) ? -1 : (pre_left_wall>number(0) ? 1 : 0);
                    right_wall= pre_right_wall<number(0) ? -1 : (pre_right_wall>number(0) ? 1 : 0);
                    bottom_wall = classify_point(point, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords);
                    top_wall = classify_point(point, trapeze.right_top->origin->coords, trapeze.left_top->origin->coords);
                    if(point==trapeze.right_top->origin->coords || point==trapeze.left_top->origin->coords) top_wall=0; // this may be more robust and accurate for boundary vertices
                    if(point==trapeze.left_bottom->origin->coords || point==trapeze.right_bottom->origin->coords) bottom_wall=0; // this may be more robust and accurate for boundary vertices
                }
                void compute_codes_from_class(const vertex& v, const point_class_with_trapeze & classs, const trapeze_t & trapeze) {
                    if(classs==point_class_with_trapeze::outside || classs==point_class_with_trapeze::unknown)
                        compute_location_codes(v, trapeze);
                    else { // it is in closure
                        left_wall=bottom_wall=right_wall=top_wall=1; // this covers strictly inside
                        if(v==trapeze.left_top->origin->coords) top_wall=left_wall=0;
                        if(v==trapeze.left_bottom->origin->coords) left_wall=bottom_wall=0;
                        if(v==trapeze.right_bottom->origin->coords) bottom_wall=right_wall=0;
                        if(v==trapeze.right_top->origin->coords) right_wall=top_wall=0;
                        if(classs==point_class_with_trapeze::left_wall) left_wall=0;
                        else if(classs==point_class_with_trapeze::bottom_wall) bottom_wall=0;
                        else if(classs==point_class_with_trapeze::right_wall) right_wall=0;
                        else if(classs==point_class_with_trapeze::top_wall) top_wall=0;
                    }
                }
                point_class_with_trapeze classify_from_location_codes(const location_codes &codes) {
                    // given any point, classify it against the location codes, this is robust
                    if(codes.isInClosure()) {
                        if(codes.isBoundaryVertex()) return point_class_with_trapeze::boundary_vertex;
                        if(codes.left_wall==0) return point_class_with_trapeze::left_wall;
                        if(codes.right_wall==0) return point_class_with_trapeze::right_wall;
                        if(codes.bottom_wall==0) return point_class_with_trapeze::bottom_wall;
                        if(codes.top_wall==0) return point_class_with_trapeze::top_wall;
                        return point_class_with_trapeze::strictly_inside;
                    }
                    return point_class_with_trapeze::outside;
                }
                static point_class_with_trapeze classOfIndex(int index) {
                    if(index==0)
                        return point_class_with_trapeze::left_wall;
                    else if(index==1)
                        return point_class_with_trapeze::bottom_wall;
                    else if(index==2)
                        return point_class_with_trapeze::right_wall;
                    else if(index==3)
                        return point_class_with_trapeze::top_wall;
                }
                static int indexOfClass(point_class_with_trapeze cls) {
                    switch (cls) {
                        case point_class_with_trapeze::left_wall:
                            return 0;
                        case point_class_with_trapeze::bottom_wall:
                            return 1;
                        case point_class_with_trapeze::right_wall:
                            return 2;
                        case point_class_with_trapeze::top_wall:
                            return 3;
                        default:
                            return 0;
                    }
                }
                bool isBoundaryVertex() {
                    if(left_wall==0&&top_wall==0) return true; // left-top
                    else if(left_wall==0&&bottom_wall==0) return true; // left-bottom
                    else if(bottom_wall==0&&right_wall==0) return true; // right-bottom
                    else if(right_wall==0&&top_wall==0) return true; // right-top
                    else return false; // else
                }
                bool isOnBoundary() {
                    return (left_wall==0 || bottom_wall==0 || right_wall==0 || top_wall==0);
                }
                bool isInterior() {
                    return (left_wall>0 && bottom_wall>0 && right_wall>0 && top_wall>0);
                }
                bool isInClosure() {
                    return isOnBoundary() || isInterior();
                }
                bool isExterior() {
                    return !isInClosure();
                }
            };


        public:
            static
            void compute(const chunker<vertex> &pieces,
                       const fill_rule &rule,
                       const tess_quality &quality,
                       dynamic_array<vertex> &output_vertices,
                       triangles::indices & output_indices_type,
                       dynamic_array<index> &output_indices,
                       dynamic_array<microgl::triangles::boundary_info> *boundary_buffer= nullptr,
                       dynamic_array<vertex> *debug_trapezes= nullptr);

        private:
            static
            void face_to_trapeze_vertices(half_edge_face * face, dynamic_array<vertex> &vertices) {
                if(face->edge== nullptr)
                    return;
                auto trapeze = infer_trapeze(face);
                vertices.push_back(trapeze.left_top->origin->coords);
                vertices.push_back(trapeze.right_top->origin->coords);
                vertices.push_back(trapeze.right_bottom->origin->coords);
                vertices.push_back(trapeze.left_bottom->origin->coords);
            }

            static void
            tessellate(half_edge_face **faces, index size, const fill_rule &rule,
                    tess_quality quality, dynamic_array<vertex> &output_vertices,
                    triangles::indices & output_indices_type,
                    dynamic_array<index> &output_indices,
                    dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                    dynamic_array<vertex> *debug_trapezes);

            static
            auto create_frame(const chunker<vertex> &pieces, dynamic_pool & dynamic_pool) -> half_edge_face *;

            static
            auto build_poly_and_conflicts(const chunker<vertex> &pieces,
                                          half_edge_face & main_frame,
                                          poly_info ** poly_list_out,
                                          conflict ** conflict_list_out) -> void ;

            static
            int classify_point(const vertex &point, const vertex &a, const vertex &b);

            static
            intersection_status finite_segment_intersection_test(const vertex &a, const vertex &b,
                                                                 const vertex &c, const vertex &d,
                                                                 vertex &intersection,
                                                                 number &alpha, number &alpha1);

            static
            auto infer_trapeze(const half_edge_face *face) -> trapeze_t;

            static
            auto handle_vertical_face_cut(const trapeze_t &trapeze,
                                          vertex & a,
                                          const point_class_with_trapeze &a_classs,
                                          dynamic_pool &pool) -> vertical_face_cut_result;

            static
            auto try_split_edge_at(const vertex& point, half_edge *edge, dynamic_pool &pool) -> half_edge * ;

            static half_edge *
            try_insert_vertex_on_trapeze_boundary_at(const vertex &v, const trapeze_t &trapeze,
                                                     point_class_with_trapeze where_boundary, dynamic_pool &pool);

            static
            number evaluate_line_at_x(number x, const vertex &a, const vertex &b);

            static
            auto
            insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge, half_edge *vertex_b_edge,
                    const vertex &extra_direction_for_split,
                    dynamic_pool &dynamic_pool) -> half_edge *;

            static
            auto classify_conflict_against_two_faces(const half_edge *face_separator,
                    const vertex &c, const vertex &d, const vertex &extra_direction_for_split={0,0})->half_edge_face *;

            static
            void re_distribute_conflicts_of_split_face(conflict *conflict_list, const half_edge *face_separator,
                    const vertex &extra_direction_for_split={0,0});

            static
            void walk_and_update_edges_face(half_edge *edge_start, half_edge_face *face);

            static
            number clamp(const number &val, number a, number b);

            static
            auto
            compute_conflicting_edge_intersection_against_trapeze(const trapeze_t &trapeze,
                                                                  vertex &a, vertex b, const point_class_with_trapeze & a_class)
                                                                  -> conflicting_edge_intersection_status;

            static
            point_class_with_trapeze
            do_a_b_lies_on_same_trapeze_wall(const trapeze_t &trapeze, const vertex &a, const vertex &b,
                                                  const point_class_with_trapeze &a_class,
                                                  const point_class_with_trapeze &b_class);

            static
            int infer_edge_winding(const vertex &a, const vertex &b);

            static
            bool is_a_before_or_equal_b_on_same_boundary(const vertex &a, const vertex &b,
                                                         const point_class_with_trapeze &wall);

            static
            void handle_co_linear_edge_with_trapeze(const trapeze_t &trapeze, half_edge *edge_vertex_a,
                                                    half_edge *edge_vertex_b,
                                                    const point_class_with_trapeze &wall_class,
                                                    int winding);

            static
            face_split_result
            handle_face_split(const trapeze_t &trapeze, vertex &a, vertex &b,
                              vertex extra_direction,
                              const point_class_with_trapeze &a_class,
                              const point_class_with_trapeze &b_class,
                              int winding,
                              dynamic_pool &dynamic_pool);

            static
            half_edge *locate_half_edge_of_face_rooted_at_vertex(const half_edge_vertex *root, const half_edge_face *face);

            static
            auto locate_face_of_a_b(const half_edge_vertex &a, const vertex &b) -> half_edge *;

            static
            bool is_distance_to_line_less_than_epsilon(const vertex &v, const vertex &a, const vertex &b, number epsilon);

            static
            void remove_edge(half_edge *edge);

            static
            void handle_face_merge(const half_edge_vertex *vertex_on_vertical_wall);

            static
            point_class_with_trapeze
            round_vertex_to_trapeze(vertex &point, const trapeze_t &trapeze);

            static
            int compute_face_windings(half_edge_face *face);

            static
            bool infer_fill(int winding, const fill_rule &rule);

            static
            half_edge *locate_next_trapeze_boundary_vertex_from(half_edge *a, const trapeze_t &trapeze);

            static
            half_edge *locate_prev_trapeze_boundary_vertex_from(half_edge *a, const trapeze_t &trapeze);

            static
            void insert_poly(poly_info &poly, dynamic_pool &dynamic_pool);

            static
            vertex_location_result
            locate_and_classify_vertex_that_is_already_on_trapeze(const half_edge_vertex *v, const trapeze_t &trapeze);

            static
            auto contract_edge(half_edge *e) -> half_edge_vertex *;

            static
            number robust_dot(const vertex &u, const vertex &v);

            static void
            wall_vertex_endpoints(const trapeze_t &trapeze, const point_class_with_trapeze &wall, vertex &start, vertex &end);

            static void
            wall_edge_endpoints(const trapeze_t & trapeze, const point_class_with_trapeze & wall,
                           half_edge ** start, half_edge ** end);
            static
            vertex clamp_vertex_to_trapeze_wall(const vertex &v, const point_class_with_trapeze &wall,
                                                const trapeze_t &trapeze);
        };

    }

}

#include "planarize_division.cpp"