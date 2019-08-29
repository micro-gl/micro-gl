#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/TriangleIndices.h>
#include <stdexcept>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class FanTriangulation {
    public:

        explicit FanTriangulation(bool DEBUG = false);;

        index * compute(vec2_f * $pts,
                        index size,
                        index *indices_buffer_triangulation,
                        index indices_buffer_size,
                        const triangles::TrianglesIndices &requested =
                                triangles::TrianglesIndices::TRIANGLES_FAN
        );

        index * compute(vec2_32i * $pts,
                        index size,
                        index *indices_buffer_triangulation,
                        index indices_buffer_size,
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