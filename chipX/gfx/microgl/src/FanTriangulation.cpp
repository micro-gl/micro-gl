#include <microgl/tesselation/FanTriangulation.h>

namespace tessellation {

    void FanTriangulation::compute(vec2_32i *$pts,
                                   index size,
                                   dynamic_array<index> & indices_buffer_triangulation,
                                   dynamic_array<triangles::boundary_info> * boundary_buffer,
                                   const triangles::TrianglesIndices &requested) {

        bool requested_triangles_with_boundary =
                requested==triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY;
        auto &indices = indices_buffer_triangulation;

        for (index ix = 0; ix < size; ++ix) {
            indices.push_back(ix);
        }

        if(requested_triangles_with_boundary) {

            const index num_triangles = size-2;

            for (index ix = 0; ix < num_triangles; ++ix) {
                bool aa_first_edge = ix==0;
                bool aa_second_edge = true;
                bool aa_third_edge = ix==num_triangles-1;

                triangles::boundary_info aa_info =
                        triangles::create_boundary_info(aa_first_edge,
                                                        aa_second_edge,
                                                        aa_third_edge);

                boundary_buffer->push_back(aa_info);

            }

        }

    }

    void
    FanTriangulation::compute(vec2_f *$pts,
                              index size,
                              dynamic_array<index> & indices_buffer_triangulation,
                              dynamic_array<triangles::boundary_info> * boundary_buffer,
                              const triangles::TrianglesIndices &requested) {

        compute((vec2_32i *)nullptr,
                size,
               indices_buffer_triangulation,
                boundary_buffer,
                requested);
    }

    index FanTriangulation::required_indices_size(const index polygon_size,
                                                  const triangles::TrianglesIndices &requested) {
        switch (requested) {
            case triangles::TrianglesIndices::TRIANGLES_FAN:
            case triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY:
                return polygon_size;
            default:
                return 0;
        }
    }

}