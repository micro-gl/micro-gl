#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>
#include <microgl/math.h>

namespace microgl {
    namespace tessellation {

        enum class stroke_cap {
            butt, round, square
        };

        enum class stroke_line_join {
            none, miter, miter_clip, round, bevel
        };

        enum class stroke_gravity {
            center, inward, outward
        };

        template<typename number>
        class stroke_tessellation {
        public:
            using vertex = vec2<number>;
            using index = unsigned int;
            using precision = unsigned char;
        private:
            struct edge {
                vertex a, b;
            };

            struct poly_4 {
                // clock wise around the bone
                vertex left, top, right, bottom;
                int left_index=-1, top_index=-1, right_index=-1, bottom_index=-1;
            };

            struct poly_inter_result {
                vertex left, right;
                int left_index=-1, right_index=-1;
                bool has_left=false, has_right=false;
            };

        public:
            static
            void compute(const number &stroke_width,
                         bool closePath,
                         const stroke_cap &cap,
                         const stroke_line_join &line_join,
                         const stroke_gravity &gravity,
                         const number &miter_limit,
                         const vertex *points,
                         index size,
                         dynamic_array<vertex> &output_vertices,
                         dynamic_array<index> &output_indices,
                         triangles::indices &output_indices_type,
                         dynamic_array<triangles::boundary_info> *boundary_buffer= nullptr);

            static
            void compute_ONE(const number &stroke_width,
                         bool closePath,
                         const stroke_cap &cap,
                         const stroke_line_join &line_join,
                         const stroke_gravity &gravity,
                         const number &miter_limit,
                         const vertex *points,
                         index size,
                         dynamic_array<vertex> &output_vertices,
                         dynamic_array<index> &output_indices,
                         triangles::indices &output_indices_type,
                         dynamic_array<triangles::boundary_info> *boundary_buffer= nullptr);

        private:

            enum class intersection_status {
                intersect, none, parallel, degenerate_line
            };

            static
            auto build_quadrilateral(const vertex &a, const vertex &b, const number &stroke_width)-> poly_4;


            static
            auto finite_segment_intersection_test(const vertex &a, const vertex &b,
                                             const vertex &c, const vertex &d,
                                             vertex & intersection,
                                             number &alpha, number &alpha1) -> intersection_status;

            static
            bool rays_intersect(const vertex &a, const vertex &b,
                    const vertex &c, const vertex &d, vertex &intersection);

            static
            void comp_parallel_ray(const vertex &pt0,
                                   const vertex &pt1,
                                   vertex &pt_out_0,
                                   vertex &pt_out_1,
                                   number stroke
            );

            static
            auto resolve_left_right_walls_intersections(const poly_4 &a, const poly_4 &b) -> poly_inter_result;

            static
            int classify_point(const vertex &point, const vertex &a, const vertex &b);

            static edge
            compute_distanced_tangent_at_joint(const vertex &a, const vertex &b, const vertex &c, number mag);

            static
            void compute_arc(const vertex &from, const vertex &root, const vertex &to,
                             const number &radius, const number &max_distance_squared,
                             const index & root_index,
                             dynamic_array<vertex> &output_vertices, dynamic_array<index> &output_indices,
                             dynamic_array<microgl::triangles::boundary_info> *boundary_buffer);

            static void
            apply_cap(const stroke_cap &cap,
                        bool is_start, const vertex &root,
                        const index &a_index, const index &b_index,
                      const number &radius, dynamic_array<vertex> &output_vertices,
                      dynamic_array<index> &output_indices,
                      dynamic_array<microgl::triangles::boundary_info> *boundary_buffer);

            static
            void apply_line_join(const stroke_line_join &line_join,
                                 const index &first_index, const index &join_index, const index &last_index,
                                 const number &join_radius, const number &miter_limit,
                                 dynamic_array<vertex> &output_vertices,
                                 dynamic_array<index> &output_indices,
                                 dynamic_array<microgl::triangles::boundary_info> *boundary_buffer= nullptr);

        };


    }

}
#include "stroke_tessellation.cpp"