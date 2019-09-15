#include <microgl/tesselation/complex_to_simple_polygon.h>

namespace tessellation {


    void complex_to_simple_polygon::compute(vec2_f *$pts,
                                            index size,
                                            array_container<vec2_f> & polygons_result,
                                            array_container<index> & polygons_locations,
                                            vector<int> winding) {
        MultiPoly multi, result_multi;

        Poly poly{};

        for (index ix = 0; ix < size; ++ix) {
            auto pt = $pts[ix];
            Vertex v{pt.x, pt.y};

            poly.vtxList.push_back(v);
        }

//        multi.m_polyList.push_back(poly);
        multi.add(poly);
        int aa= multi.m_polyList.back().size();
        int aa1= multi.m_polyList.front().size();
        multi.findMonotone(result_multi, winding);

        auto & poly_list = result_multi.m_polyList;
        int ref_size = polygons_result.size();

        for (auto &p : poly_list) {
            for (auto &v : p.vtxList) {
                polygons_result.push_back({v.x, v.y});
            }
            // record the polygons
            polygons_locations.push_back(polygons_result.size() - ref_size);
            ref_size += polygons_result.size();
        }

        int a =0;
    }

    void complex_to_simple_polygon::compute(vec2_32i *$pts,
                                            index size) {

    }
}