#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include "micro_tess_traits.h"

namespace microgl {
    namespace tessellation {

        template<typename number, class container_output_indices, class container_output_boundary>
        class fan_triangulation {
            using index = unsigned int;
            using vertex = vec2<number>;
        public:

            static
            void compute(const vertex *points,
                         index size,
                         container_output_indices &indices_buffer_triangulation,
                         container_output_boundary *boundary_buffer,
                         triangles::indices &output_type) {

                typename microtess::traits::enable_if<
                        microtess::traits::is_same<typename container_output_indices::value_type,
                        index>::value, bool>::type is_indices_container_valid;
                typename microtess::traits::enable_if<
                        microtess::traits::is_same<
                                typename container_output_boundary::value_type, triangles::boundary_info>::value
                        , bool>::type is_boundary_container_valid;


                bool requested_triangles_with_boundary = boundary_buffer;
                output_type=requested_triangles_with_boundary? microgl::triangles::indices::TRIANGLES_FAN_WITH_BOUNDARY :
                            microgl::triangles::indices::TRIANGLES_FAN;
                auto &indices = indices_buffer_triangulation;

                for (index ix = 0; ix < size; ++ix)
                    indices.push_back(ix);

                if (requested_triangles_with_boundary) {
                    const index num_triangles = size - 2;
                    for (index ix = 0; ix < num_triangles; ++ix) {
                        bool aa_first_edge = ix == 0;
                        bool aa_second_edge = true;
                        bool aa_third_edge = ix == num_triangles - 1;
                        microgl::triangles::boundary_info aa_info =
                                microgl::triangles::create_boundary_info(aa_first_edge,
                                                                aa_second_edge,
                                                                aa_third_edge);
                        boundary_buffer->push_back(aa_info);
                    }

                }

            }

        };

    }

}