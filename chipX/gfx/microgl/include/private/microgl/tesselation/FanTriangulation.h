#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/array_container.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class FanTriangulation {
    public:

        explicit FanTriangulation(bool DEBUG = false);;

        static
        void compute(vec2_f * $pts,
                     index size,
                     array_container<index> & indices_buffer_triangulation,
                     array_container<triangles::boundary_info> * boundary_buffer,
                     const triangles::TrianglesIndices &requested =
                                triangles::TrianglesIndices::TRIANGLES_FAN
        );

        static
        void compute(vec2_32i * $pts,
                     index size,
                     array_container<index> & indices_buffer_triangulation,
                     array_container<triangles::boundary_info> * boundary_buffer,
                     const triangles::TrianglesIndices &requested =
                                triangles::TrianglesIndices::TRIANGLES_FAN
                        );

        /**
         * computes the required indices buffer size for requested triangulation
         *
         * @param polygon_size
         * @param requested
         * @return
         */
        static index required_indices_size(index polygon_size,
                                           const triangles::TrianglesIndices &requested =
                                           triangles::TrianglesIndices::TRIANGLES_FAN);

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop