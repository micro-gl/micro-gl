#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>
#include <microgl/math.h>

namespace microgl {
    namespace tessellation {

        enum class path_gravity {
            center, inward, outward
        };

        template<typename number>
        class path_tessellation {
        public:
            using vertex = vec2<number>;
            using index = unsigned int;
            using precision = unsigned char;

            static
            void compute(number stroke_size,
                         bool closePath,
                         path_gravity gravity,
                         const vertex *points,
                         index size,
                         dynamic_array<index> &indices_buffer_tessellation,
                         dynamic_array<vertex> &output_vertices_buffer_tessellation,
                         dynamic_array<triangles::boundary_info> *boundary_buffer,
                         const triangles::indices &requested =
                         triangles::indices ::TRIANGLES_STRIP
            );

        private:
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


        };


    }

}
#include "path_tessellation.cpp"