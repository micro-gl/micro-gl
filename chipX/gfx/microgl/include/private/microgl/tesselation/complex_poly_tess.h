#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/chunker.h>
#include <microgl/tesselation/simple_components_tree.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class complex_poly_tess {
    public:

        explicit complex_poly_tess(bool DEBUG = false) {};

        static void compute(chunker<microgl::vec2_f> & pieces,
                            dynamic_array<microgl::vec2_f> & result,
                            dynamic_array<microgl::vec2_f> & indices
                            );

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop