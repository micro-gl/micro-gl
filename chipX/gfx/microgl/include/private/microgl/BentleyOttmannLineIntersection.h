#pragma once

#include <microgl/Types.h>
//#include "RedBlackTree.h"
#include "AVLTree2.h"
#include <vector>
#include <iostream>
#include <stdexcept>

namespace tessellation {

//    http://www.normalesup.org/~cagne/internship_l3/mncubes_red/doc/html/bentley__ottmann_8cpp_source.html

#define max(a,b) (a)>(b) ? (a) : (b)
#define min(a,b) (a)<(b) ? (a) : (b)
#define clamp(a,l,r) (min(max((a), (l)), (r)))
#define abs(a) ((a)<0 ? -(a) : (a))
#define E 1.0f

    enum class event_type_t {
        START, END, Intersection
    };

    struct computation_t {
        int numerator, denominator;
    };

    struct segment_t {
        vec2_32i p0, p1;

        computation_t compute_slope_parts() const {
            return {p1.x - p0.x, p1.y - p0.y};
        }

        computation_t compute_x_intersection_with(const vec2_32i & p) const {
            computation_t c {0, 0};

            if (p0.y==p1.y) {
                c.numerator = clamp(p.x, p0.x, p1.x);
                c.denominator = 1;
            } else {
                auto dx = p1.x - p0.x;
                auto dy = p1.y - p0.y;

                c.numerator = p0.x*dy + (p.y - p0.y)*dx;
                c.denominator = dy;
            }

            return c;
        }

        bool contains(const vec2_32i & p, float epsilon = 1.0) {

            // this is an optimization to avoid divisions
            auto dx = p1.x - p0.x;
            auto dy = p1.y - p0.y;

            bool lies_on_ray = false;

            if(dy!=0) {
                auto M = p0.x*dy + (p.y - p0.y)*dx;
                auto xDistance = abs(p.x*dy - M);
//                lies_on_ray = xDistance < epsilon*dy;
                lies_on_ray = xDistance < 3*(abs(dx) + abs(dy));
            } else {
                lies_on_ray = p.y==p0.y;
            }

            if(dy == 0) {
//                contains =
            }

            // check edge cases beyond the segment
            if(lies_on_ray) {
                auto minX = min(p0.x, p1.x);
                auto minY = min(p0.y, p1.y);
                auto maxX = max(p0.x, p1.x);
                auto maxY = max(p0.y, p1.y);

                bool outside_x = p.x < minX || p.x > maxX;
                bool outside_y = p.y < minY || p.y > maxY;

                if(outside_x || outside_y)
                    return false;
            }

            return lies_on_ray;

        }

        bool isStart(const vec2_32i & p) {
            return p0.x==p.x && p0.y==p.y;
        }

        bool isEnd(const vec2_32i & p) {
            return p1.x==p.x && p1.y==p.y;
        }

    };

    struct event_point_t {
        // experimenting

        segment_t segment{};
        event_type_t type = event_type_t::START;

        const vec2_32i & getPoint() const {
            switch (type) {
                case event_type_t::START:
                    return segment.p0;
                case event_type_t::END:
                    return segment.p1;
                default:
                    return segment.p0;
            }

        }

    };

    struct event_order
    {
        bool isPreceding(const event_point_t& lhs, const event_point_t& rhs)
        {
            return (lhs.getPoint().y < rhs.getPoint().y) ||
                   ((lhs.getPoint().y == rhs.getPoint().y) && (lhs.getPoint().x < rhs.getPoint().x));
        }

        bool isPrecedingOrEqual(const event_point_t& lhs, const event_point_t& rhs)
        {
            return isPreceding(lhs, rhs) || isEqual(lhs, rhs);
        }

        bool isEqual(const event_point_t& lhs, const event_point_t& rhs)
        {
            return (lhs.getPoint().x==rhs.getPoint().x) && (lhs.getPoint().y==rhs.getPoint().y);
        }

        bool isExact(const event_point_t& lhs, const event_point_t& rhs)
        {
            return (lhs.getPoint().x==rhs.getPoint().x) && (lhs.getPoint().y==rhs.getPoint().y);
        }
    };

    struct segment_order
    {
        int backup(const segment_t& lhs, const segment_t& rhs) {
            //
            // extended intersection without division
            auto dx_1 = lhs.p1.x - lhs.p0.x;
            auto dy_1 = lhs.p1.y - lhs.p0.y;

            auto dx_2 = rhs.p1.x - rhs.p0.x;
            auto dy_2 = rhs.p1.y - rhs.p0.y;

            auto M1 = 0;
            auto M2 = 0;
            auto m1 = 0;
            auto m2 = 0;

            if(dy_1!=0) {
                m1 = dy_1;
                M1 = lhs.p0.x*dy_1 + (p.y - lhs.p0.y)*dx_1;
            }

            if(dy_2!=0) {
                m2 = dy_2;
                M2 = rhs.p0.x*dy_2 + (p.y - rhs.p0.y)*dx_2;
            }

            // special cases
            // lhs is a line
            // both are lines or points
            if(dy_1==0 && dy_2==0 && lhs.p0.y==p.y && rhs.p0.y==p.y) {
                // if they intersect
                auto left = max(lhs.p0.x, rhs.p0.x);
                auto right = min(lhs.p1.x, rhs.p1.x);

                // if they intersect, they are the same
                if((right-left) >= 0) {
                    M1=m1=M2=m2 = 1;
                }

            }
                // if one of them is a horizontal line
            else {
                if(dy_1==0 && lhs.p0.y==p.y) {
                    // lies on p.y
                    M1 = clamp(M2, m2*lhs.p0.x, m2*lhs.p1.x);
                    m1 = m2;

                    // a point
                    if(dx_1==0) {
                        m1 = 1;
                        M1 = lhs.p0.x;
                    }
                }
                else if(dy_2==0 && rhs.p0.y==p.y) {
                    // lies on p.y
                    M2 = clamp(M1, m1*rhs.p0.x, m1*rhs.p1.x);
                    m2 = m1;

                    // a point
                    if(dx_2==0) {
                        m2 = 1;
                        M2 = rhs.p0.x;
                    }
                }
            }



            // we want to represent
            // x_left = M1/m1, x_right = M2/m2
            // compare_result = x_left - x_right

            int pre = M1 * m2 - M2 * m1;

            if(abs(pre) <= 3 * abs(m1*m2))
                return 0;
//            int pre = M1/m1 - M2/m2;

            return pre;
        }


        int cmp(const segment_t& lhs, const segment_t& rhs) {

            computation_t lhs_x = lhs.compute_x_intersection_with(p);
            computation_t rhs_x = rhs.compute_x_intersection_with(p);

            // this is to avoid divisions, which are expensive
            int compare = lhs_x.numerator*rhs_x.denominator - rhs_x.numerator*lhs_x.denominator;

            if(compare!=0)
                return compare;

            // if they are equal at p, we need to break tie with slope
            computation_t lhs_slope = lhs.compute_slope_parts();
            computation_t rhs_slope = rhs.compute_slope_parts();

            bool lhs_has_infinite_slope = lhs_slope.denominator==0;
            bool rhs_has_infinite_slope = rhs_slope.denominator==0;

            if(lhs_has_infinite_slope)
                return 1;

            if(rhs_has_infinite_slope)
                return -1;

            int slope_compare = lhs_slope.numerator * rhs_slope.denominator -
                                rhs_slope.numerator * lhs_slope.denominator;

            return slope_compare;
        }

        bool isPreceding(const segment_t& lhs, const segment_t& rhs)
        {
            return cmp(lhs, rhs) < 0;
        }

        bool isPrecedingOrEqual(const segment_t& lhs, const segment_t& rhs)
        {
            return cmp(lhs, rhs) <= 0;
        }

        bool isEqual(const segment_t& lhs, const segment_t& rhs)
        {
            return cmp(lhs, rhs) == 0;
        }

        bool isExact(const segment_t& lhs, const segment_t& rhs)
        {
            return
                lhs.p0.x==rhs.p0.x && lhs.p1.x==rhs.p1.x &&
                lhs.p0.y==rhs.p0.y && lhs.p1.y==rhs.p1.y;
        }

        void updateComparePoint(const vec2_32i & val) {
            p = val;
        }
    private:

        vec2_32i p;
    };

    // Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
    // intersect the intersection point may be stored in the floats i_x and i_y.
    bool get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
                               float p2_x, float p2_y, float p3_x, float p3_y,
                               float *i_x=nullptr, float *i_y=nullptr)
    {
        float s1_x, s1_y, s2_x, s2_y;
        s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
        s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

        float s, t;
        s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
        t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

        if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
        {
            // Collision detected
            if (i_x != nullptr)
                *i_x = p0_x + (t * s1_x);
            if (i_y != nullptr)
                *i_y = p0_y + (t * s1_y);
            return true;
        }

        return false; // No collision
    }

    class BentleyOttmann {
    public:
        BentleyOttmann(vec2_32i * pts, int size) {
            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(pts[ix], pts[ix+1], event_type_t::START);
                event_point_t e2 = create_event(pts[ix], pts[ix+1], event_type_t::END);
                Queue.insert(e1);
                Queue.insert(e2);
            }
        }

        BentleyOttmann(segment_t * seg, int size) {
            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(seg[ix], event_type_t::START);
                event_point_t e2 = create_event(seg[ix], event_type_t::END);
                Queue.insert(e1);
                Queue.insert(e2);
            }
        }

        event_point_t create_event(const vec2_32i & p0, const vec2_32i & p1, event_type_t type) {
            event_point_t pt;
            pt.segment.p0 = p0;
            pt.segment.p1 = p1;
//            pt.segment.p0 = p0.y<=p1.y ? p0 : p1;
//            pt.segment.p1 = p0.y<=p1.y ? p1 : p0;
            pt.type = type;

            return pt;
        }

        event_point_t create_event(const segment_t & segment, event_type_t type) {
            return create_event(segment.p0, segment.p1, type);
        }

        void line_intersection() {
            while (!Queue.isEmpty()) {
                event_point_t event = Queue.removeMinKey();

                /*
                // do this later to handle multiple
                while (Q.contains(event)) {
                    event_point_t event2 = Q.removeMinElement();
                    // insert this segment into event
                }
                */
                std::string info = std::to_string(event.getPoint().x) +
                                    "x" + std::to_string(event.getPoint().y);
                std::cout<<"event p " << info << std::endl;
                handleEventPoint(event);
            }

        }

        void handleEventPoint(event_point_t & event) {
            unsigned int score = 0;
            std::vector<segment_t> U_p {};

            // for now suppose U(p) is always a singleton,
            // I will solve it later
            if(event.type==event_type_t::START)
                U_p.push_back(event.segment);

            auto & p = event.getPoint();

            // update the scanning y
            S.getComparator().updateComparePoint(p);

            //
            // 2. find all segments that contain p, and classify them
            //
            std::vector<segment_t> L_p, C_p;
            // create a zero length segment at p
            segment_t p_segment{p, p};
//            Status::Node * node = S.findUpperBoundOf(p_segment);
            Status::Node * node = S.findLowerBoundOf(p_segment);

            while(node!=nullptr) {
                std::cout << "check 1" <<std::endl;

                segment_t & tested_segment = node->key;//.contains(p)
                bool tested_segment_contains_p =
                        tested_segment.contains(p);

                if(tested_segment_contains_p) {
                    // segment contains p, now let's classify
                    // first check if p is at the end
                    if(tested_segment.isEnd(p))
                        L_p.push_back(node->key);
                    else if(!tested_segment.isStart(p)) {
                        // if it is not start or end, then it is
                        // strictly inside
                        C_p.push_back(node->key);
                    }

                } else {
                    // if segment does not contain p, then no
                    // further segment will contain, so break the search
                    // THIS MIGHT BE A PROBLEM
                    break;
                }

                node = S.predecessor(node);
            }

            // end classification

            // 3. test intersection and report


            if((U_p.size() + L_p.size() + C_p.size()) > 1) {
                // report intersection
                std::cout << "intersection with p : " + std::to_string(p.x) +
                            ", " + std::to_string(p.y) << std::endl;

                I.push_back({p.x, p.y});
            }

            // 5. delete segments in L_p, C_p from Status
            for (auto & ix : L_p) {
                S.remove(ix);
            }

            for (auto & ix : C_p) {
                S.remove(ix);
            }

            // 6. insert U_p and C_p
            for (auto & ix : U_p) {
                S.insert(ix);
            }

            for (auto & ix : C_p) {
                S.insert(ix);
            }

            // 8
            bool is_Up_or_C_p_empty = U_p.empty() && C_p.empty();

            if(is_Up_or_C_p_empty) {
                // create a zero length segment at p
//                throw std::invalid_argument( "implement" );
                // this requires more thinking
                segment_t p_segment{p, p};
                Status::Node * right = S.findLowerBoundOf(p_segment);

                if(right!= nullptr) {
                    Status::Node * left = S.predecessor(right);

                    if(left!= nullptr)
                        find_new_event(left->key, right->key, p);
                }

            }
            else {

                // find left-most and right-most segments in U_p + C_p
                segment_order order{};
                order.updateComparePoint(p);

                segment_t min{}, max{};

                if(!U_p.empty())
                    min = max = U_p[0];
                else
                    min = max = C_p[0];

                // this is replicating the order in which they
                // are inserted in the status tree
                for (auto & ix : U_p) {
                    if(order.isPreceding(ix, min))
                        min = ix;

                    if(!order.isPreceding(ix, max))
                        max = ix;
                }

                for (auto & ix : C_p) {
                    if(order.isPreceding(ix, min))
                        min = ix;

                    if(!order.isPreceding(ix, max))
                        max = ix;

                }

                // now find it's left neighbor in Status
                Status::Node * min_node = S.searchExact(min);
                Status::Node * left_neighbor = S.predecessor(min_node);

                if(left_neighbor!= nullptr)
                    find_new_event(left_neighbor->key, min, p);

                // second

                // now find it's left neighbor in Status
                Status::Node * max_node = S.searchExact(max);
                Status::Node * right_neighbor = S.successor(max_node);

                if(right_neighbor!= nullptr)
                    find_new_event(max, right_neighbor->key, p);
            }


        }

        void find_new_event(const segment_t & l,
                            const segment_t & r,
                            const vec2_32i & p) {
            float x, y;
            bool intersects = get_line_intersection(
                    l.p0.x, l.p0.y, l.p1.x, l.p1.y,
                    r.p0.x, r.p0.y, r.p1.x, r.p1.y,
                    &x, &y
                    );

            if(!intersects)
                return;

            // this is important for now, otherwise
            // we won't discard if it happened above us
            x = int(x);
            y = int(y);

            bool below_p_line = y > p.y;
            bool on_p_and_right = (abs(y-p.y)<E) && (x >= p.x);

            if(below_p_line || on_p_and_right) {
                event_point_t event;
                event.segment.p0 = {(int)x, (int)y};
                event.segment.p1 = {(int)x, (int)y};
                event.type = event_type_t::Intersection;

                if(Queue.search(event)!= nullptr)
                    return;

                Queue.insert(event);
            }

        }

        std::vector<vec2_32i> & getIntersections() {
            return I;
        }

    private:
        typedef ds::AVLTree2<event_point_t, event_order> PriorityQueue;
        typedef ds::AVLTree2<segment_t, segment_order> Status;

        PriorityQueue Queue;
        Status S;
        std::vector<vec2_32i> I;

    };

}