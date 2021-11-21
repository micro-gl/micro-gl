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
#include "traits.h"

namespace microtess {
    /**
     * Fan triangulation
     * @tparam number the number type of a vertex
     * @tparam container_output_indices the output indices container type
     * @tparam container_output_boundary the output boundary container type
     */
    template<typename number, class container_output_indices,
            class container_output_boundary>
    class fan_triangulation {
    public:
        using index = unsigned int;
        using vertex = microtess::vec2<number>;

        fan_triangulation()=delete;
        fan_triangulation(const fan_triangulation &)=delete;
        fan_triangulation(fan_triangulation &&)=delete;
        fan_triangulation & operator=(const fan_triangulation &)=delete;
        fan_triangulation & operator=(fan_triangulation &&)=delete;
        ~fan_triangulation()=delete;

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
            output_type=requested_triangles_with_boundary? triangles::indices::TRIANGLES_FAN_WITH_BOUNDARY :
                        triangles::indices::TRIANGLES_FAN;
            auto &indices = indices_buffer_triangulation;

            for (index ix = 0; ix < size; ++ix)
                indices.push_back(ix);

            if (requested_triangles_with_boundary) {
                const index num_triangles = size - 2;
                for (index ix = 0; ix < num_triangles; ++ix) {
                    bool aa_first_edge = ix == 0;
                    bool aa_second_edge = true;
                    bool aa_third_edge = ix == num_triangles - 1;
                    triangles::boundary_info aa_info =
                            triangles::create_boundary_info(aa_first_edge,
                                                            aa_second_edge,
                                                            aa_third_edge);
                    boundary_buffer->push_back(aa_info);
                }
            }
        }
    };
}