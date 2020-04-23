#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>
#include <microgl/math.h>

namespace microgl {
    namespace tessellation {

        enum class stroke_gravity {
            center, inward, outward
        };

        template<typename number>
        class stroke_tessellation {
        public:
            using vertex = vec2<number>;
            using index = unsigned int;
            using precision = unsigned char;

            struct poly_4 {
                // clock wise around the bone
                vertex left, top, right, bottom;
            };

            struct poly_inter_result {
                vertex left, right;
                bool has_left=false, has_right=false;
            };

            static
            void compute(number stroke_size,
                         bool closePath,
                         stroke_gravity gravity,
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
            bool non_parallel_rays_intersect(
                    const vertex &a,
                    const vertex &b,
                    const vertex &c,
                    const vertex &d,
                    vertex &intersection
            );

            static
            void comp_parallel_ray(const vertex &pt0,
                                   const vertex &pt1,
                                   vertex &pt_out_0,
                                   vertex &pt_out_1,
                                   number stroke
            );

            static
            void merge_intersection(
                    const vertex &a,
                    const vertex &b,
                    const vertex &c,
                    const vertex &d,
                    vertex &merge_out
            );


            static
            auto resolve_left_right_walls_intersections(const poly_4 &a, const poly_4 &b) -> poly_inter_result;
        };


    }

}
#include "stroke_tessellation.cpp"