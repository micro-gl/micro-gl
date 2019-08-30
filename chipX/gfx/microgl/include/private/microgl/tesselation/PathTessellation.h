#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/TriangleIndices.h>
#include <microgl/array_container.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class PathTessellation {
    public:

        explicit PathTessellation(bool DEBUG = false);;

        void compute(vec2_f * $pts,
                        index size,
                        array_container<index> & indices_buffer_tesselation,
                        array_container<vec2_32i> & output_vertices_buffer_tessellation,
                        const triangles::TrianglesIndices &requested =
                                triangles::TrianglesIndices::TRIANGLES_STRIP
        );

        void compute(vec2_32i * $pts,
                     index size,
                     array_container<index> & indices_buffer_tesselation,
                     array_container<vec2_32i> & output_vertices_buffer_tessellation,
                     const triangles::TrianglesIndices &requested =
                            triangles::TrianglesIndices::TRIANGLES_STRIP
                        );

        /**
         * computes the required indices buffer size for requested triangulation
         *
         * @param polygon_size
         * @param requested
         * @return
         */
        static index required_indices_size(index path_size,
                                           const triangles::TrianglesIndices &requested =
                                           triangles::TrianglesIndices::TRIANGLES_STRIP);

        /**
         * computes the required indices buffer size for requested triangulation
         *
         * @param polygon_size
         * @param requested
         * @return
         */
        static index required_vertices_size(index path_size);

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop