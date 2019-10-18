#pragma once

//#include <microgl/vec2.h>
#include <microgl/triangles.h>
//#include <microgl/chunker.h>
#include <microgl/tesselation/simple_components_tree.h>
#include <microgl/tesselation/ear_clipping_triangulation.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;

    template <typename number>
    class complex_poly_tess {
    public:
        using vertex = microgl::vec2<number>;

        static void compute(chunker<vertex> & pieces,
                            dynamic_array<vertex> & result,
                            dynamic_array<index> & indices,
                            const microgl::triangles::TrianglesIndices &requested = microgl::triangles::TrianglesIndices::TRIANGLES,
                            dynamic_array<microgl::triangles::boundary_info> * boundary_buffer = nullptr
                            );

    private:

    };


}

#include "../../src/complex_poly_tess.cpp"
