#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/linked_list.h>
#include <microgl/array_container.h>
#include <microgl/tesselation/nzw/Multipolygon.h>
#include <stdexcept>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class complex_to_simple_polygon {
    public:

        explicit complex_to_simple_polygon(bool DEBUG = false) {};

        static void compute(vec2_f * $pts,
                            index size,
                            array_container<vec2_f> & polygons_result,
                            array_container<index> & polygons_locations,
                            vector<int> winding
        );

        static void compute(vec2_32i * $pts,
                        index size
                        );

    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop