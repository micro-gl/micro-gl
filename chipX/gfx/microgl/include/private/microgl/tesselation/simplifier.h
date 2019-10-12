#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/chunker.h>
#include <microgl/tesselation/nzw/simplify_components.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class simplifier {
    public:

        explicit simplifier(bool DEBUG = false) {};

        static void compute(chunker<microgl::vec2_f> & pieces,
                            chunker<microgl::vec2_f> & result
                            );

//        static void compute(vec2_32i * $pts,
//                        index size
//                        );

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop