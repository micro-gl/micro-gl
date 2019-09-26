#include <microgl/tesselation/nzw/simplify_components.h>

namespace tessellation {

    bool compare_vertices(const vertex &v1, const vertex &v2) {
        return v1.x < v2.x || (v1.x == v2.x && v1.y < v2.y);
    }

    intersection::intersection(vertex *vtx, float p1, float p2, const segment &li, const segment &lj) {
        v = vtx;
        index1 = -1;
        index2 = -1;
        param1 = p1;
        param2 = p2;
        l1 = li;
        l2 = lj;
    }

    bool intersection::operator<(const intersection &i) const {
        return compare_vertices(*v, *i.v);
    }

    vertex *intersection::origin1() {
        return l1.start();
    }

    vertex *intersection::origin2() {
        return l2.start();
    }

    edge_vertex::edge_vertex(vertex *vtx, float p, int i) {
        v = vtx;
        param = p;
        index = i;
    }

    bool edge_vertex::operator<(const edge_vertex &n) const { return (param < n.param); }

    bool edge::operator<(const edge &s) const {
        return compare_vertices(*(vertices.begin()->v), *(s.vertices.begin()->v));
//    return ( *(ilist.begin()->v) < *(s.ilist.begin()->v) );
    }


    const float segment::NOISE = 1e-5f;
#define abs(x) ((x)<0 ? -(x) : (x))

    segment::IntersectionType segment::calcIntersection
            (const segment &l, vertex &intersection, float &alpha, float &alpha1) {
        // endpoints
        auto a = *(this->start());
        auto b = *(this->end());
        auto c = *(l.start());
        auto d = *(l.end());

        // vectors
        auto ab = b - a;
        auto cd = d - c;

        auto dem = ab.x * cd.y - ab.y * cd.x;

        // parallel lines
        // todo:: revisit when thinking about fixed points
        if (abs(dem) < NOISE)
            return PARALLEL;
        else {
            auto ca = a - c;
            auto ac = -ca;

            alpha = (ca.y * cd.x - ca.x * cd.y) / dem;
            alpha1 = (ab.y * ac.x - ab.x * ac.y) / dem;

            intersection = a + ab * alpha;

            // test for segment intersecting (alpha)
            if ((alpha < 0.0) || (alpha > 1.0))
                return NO_INTERSECT;
            else {
                float num = ca.y * ab.x - ca.x * ab.y;

                if (dem > 0.0) {
                    if (num < 0.0 || num > dem)
                        return NO_INTERSECT;
                } else {
                    if (num > 0.0 || num < dem)
                        return NO_INTERSECT;
                }
            }
        }
        return INTERSECT;
    }

    void segment::sortVertices() {
        m_swappedVertices = !(compare_vertices(*vertex0, *vertex1));
        if (m_swappedVertices)
            swap(vertex0, vertex1);
    }

    segment::segment(vertex *vtx0, vertex *vtx1) : vertex0(vtx0), vertex1(vtx1) {}

    bool segment::operator<(const segment &ls) const {
        return compare_vertices(*vertex0, *(ls.vertex0));
//    return *vertex0 < *(ls.vertex0);
    }

    bool segment::has_mutual_endpoint(const segment &a) {
        bool a_start_start = a.start()->x==this->start()->x && a.start()->y==this->start()->y;
        bool a_start_end = a.start()->x==this->end()->x && a.start()->y==this->end()->y;

        if(a_start_start || a_start_end)
            return true;
        else {
            bool a_end_start = a.end()->x==this->start()->x && a.end()->y==this->start()->y;
            bool a_end_end = a.end()->x==this->end()->x && a.end()->y==this->end()->y;

            return a_end_start || a_end_end;
        }

//        if(*a.start()==*this->start() || *a.start()==*this->end())
//            return true;
//        return *a.end() == *this->start() || *a.end() == *this->end();
    }

    segment::bbox_axis segment::classify_aligned_segment_relative_to(const segment &a, bool compare_x) {
        auto min_a = compare_x ? a.start()->x : a.start()->y;
        auto max_a = compare_x ? a.end()->x : a.end()->y;
        auto min_me = compare_x ? this->start()->x : this->start()->y;
        auto max_me = compare_x ? this->end()->x : this->end()->y;

        if(min_a > max_a) {
            auto temp = min_a;
            min_a = max_a;
            max_a = temp;
        }

        if(min_me > max_me) {
            auto temp = min_me;
            min_me = max_me;
            max_me = temp;
        }

        if(max_me <= min_a)
            return bbox_axis::start_of;
        else if(min_me >= max_a)
            return bbox_axis::end_of;
        else
            return bbox_axis::overlaps;
    }

    segment::bbox_axis segment::classify_vertical(const segment &a) {
        return classify_aligned_segment_relative_to(a, false);
    }

    segment::bbox_axis segment::classify_horizontal(const segment &a) {
        return classify_aligned_segment_relative_to(a, true);
    }

    bool segment::is_bbox_overlapping_with(const segment &a) {
        return classify_aligned_segment_relative_to(a, true)==bbox_axis::overlaps &&
               classify_aligned_segment_relative_to(a, false)==bbox_axis::overlaps;
    }

    vertex *segment::start() {
        return !m_swappedVertices ? vertex0 : vertex1;
    }

    vertex *segment::end() {
        return !m_swappedVertices ? vertex1 : vertex0;
    }

    vertex *segment::start() const {
        return !m_swappedVertices ? vertex0 : vertex1;
    }

    vertex *segment::end() const {
        return !m_swappedVertices ? vertex1 : vertex0;
    }

}