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

    struct rational_t {
        int numerator, denominator;

        float toFloat() {
            return float(numerator)/float(denominator);
        };

        int toFixed(uint8_t precision = 0) {
            return (numerator<<precision)/(denominator);
        };

        rational_t & absolute() {
            if(isNegative()) {
                numerator = numerator<0 ? -numerator : numerator;
                denominator = denominator<0 ? -denominator : denominator;
            }

            return *this;
        }

        void makeDenominatorPositive() {
            if(denominator < 0) {
                numerator = -numerator;
                denominator = -denominator;
            }

        }

        bool operator!=(const int & val) {
            return !(*this==val);
        }

        bool operator==(const int & val) {
            if(val!=0)
                return numerator==val*denominator;

            return numerator==0;
        }

        rational_t operator*(const int & val) {
            int n = numerator*val;
            return rational_t{n, denominator};
        }

        rational_t operator*(const rational_t & val) {
            int n = numerator*val.numerator;
            int d = denominator*val.denominator;
            return rational_t{n, d};
        }

        rational_t operator/(const int & val) {
            return rational_t{numerator, denominator*val};
        }

        rational_t operator/(const rational_t & val) {
            int n = numerator*val.denominator;
            int d = denominator*val.numerator;
            return rational_t{n, d};
        }

        rational_t operator+(const int & val) {
            int n = numerator + denominator*val;
            return rational_t{n, denominator};
        }

        rational_t operator+(const rational_t & val) {
            int n = numerator*val.denominator + val.numerator*denominator;
            int d = denominator * val.denominator;
            return rational_t{n, d};
        }

        rational_t operator-(const int & val) {
            int n = numerator - denominator*val;
            return rational_t{n, denominator};
        }

        rational_t operator-(const rational_t & val) {
            int n = numerator*val.denominator - val.numerator*denominator;
            int d = denominator * val.denominator;
            return rational_t{n, d};
        }

        bool operator >= (const int & val) {
            if(val!=0)
                return numerator >= denominator*val;
            else {
                return isPositive();
            }
        }

        bool operator <= (const int & val) {
            if(val!=0)
                return numerator <= denominator*val;
            else {
                return isNegative();
            }
        }

        bool isPositive() {
            return (numerator>=0 && denominator>=0)||
                   (numerator<=0 && denominator<=0);
        }

        bool isNegative() {
            return !isPositive();
        }

    };

    struct segment_t {
        vec2_32i p0, p1;

        rational_t compute_slope_parts() const {
            return {p1.x - p0.x, p1.y - p0.y};
        }

        bool contains(const vec2_32i & p) {

            // this is an optimization to avoid divisions
            auto dx = p1.x - p0.x;
            auto dy = p1.y - p0.y;
            rational_t slope{dx, dy};

            bool lies_on_ray;

            if(dy!=0) {
                /*
                auto M = p0.x*dy + (p.y - p0.y)*dx;
                auto xDistance = abs(p.x*dy - M);
                lies_on_ray = xDistance < dy;
                 */

                rational_t x = slope*(p.y - p0.y) + p0.x;
                rational_t dist = x - p.x;
                lies_on_ray = (dist.absolute() <= 1);

            } else {
                lies_on_ray = p.y==p0.y;
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
        rational_t compute_x_intersection_with(const segment_t& segment, const vec2_32i & p) const {
            // we presume we already have intersection with p.y
            auto dx = segment.p1.x - segment.p0.x;
            auto dy = segment.p1.y - segment.p0.y;
            rational_t slope{dx, dy};

            rational_t x {0, 0};

            if (dy==0) {
                x.numerator = clamp(p.x, segment.p0.x, segment.p1.x);
                x.denominator = 1;
            } else {

                x = slope*(p.y - segment.p0.y) + segment.p0.x;

//                c.numerator = segment.p0.x*dy + (p.y - segment.p0.y)*dx;
//                c.denominator = dy;
            }

            return x;
        }

        int cmp(const segment_t& lhs, const segment_t& rhs) {

            rational_t lhs_x = compute_x_intersection_with(lhs, p);
            rational_t rhs_x = compute_x_intersection_with(rhs, p);

            lhs_x.makeDenominatorPositive();
            rhs_x.makeDenominatorPositive();

            // this is to avoid divisions, which are expensive
//            int compare = lhs_x.numerator*rhs_x.denominator - rhs_x.numerator*lhs_x.denominator;
            rational_t compare = (lhs_x - rhs_x);

            if(compare!=0)
                return compare.numerator;

            // if they are equal at p, we need to break tie with slope
            rational_t lhs_slope = lhs.compute_slope_parts();
            rational_t rhs_slope = rhs.compute_slope_parts();

            lhs_slope.makeDenominatorPositive();
            rhs_slope.makeDenominatorPositive();

            bool lhs_has_infinite_slope = lhs_slope.denominator==0;
            bool rhs_has_infinite_slope = rhs_slope.denominator==0;

            if(lhs_has_infinite_slope)
                return 1;

            if(rhs_has_infinite_slope)
                return -1;

            rational_t slope_compare = lhs_slope - rhs_slope;

            return slope_compare.numerator;
        }

        bool isPreceding(const segment_t& lhs, const segment_t& rhs)
        {
            return cmp(lhs, rhs) < 0;
        }

//        bool isPrecedingOrEqual(const segment_t& lhs, const segment_t& rhs)
//        {
//            return cmp(lhs, rhs) <= 0;
//        }
//
//        bool isEqual(const segment_t& lhs, const segment_t& rhs)
//        {
//            return cmp(lhs, rhs) == 0;
//        }

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

    bool get_line_intersection_rational(int p0_x, int p0_y, int p1_x, int p1_y,
                                        int p2_x, int p2_y, int p3_x, int p3_y,
                                        rational_t *x=nullptr, rational_t *y=nullptr)
    {
        int s1_x = p1_x - p0_x;
        int s1_y = p1_y - p0_y;
        int s2_x = p3_x - p2_x;
        int s2_y = p3_y - p2_y;

        rational_t s{}, t{};
        s.numerator = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
        s.denominator = (-s2_x * s1_y + s1_x * s2_y);

        t.numerator = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
        t.denominator = (-s2_x * s1_y + s1_x * s2_y);

        if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
        {
            *x = (t * s1_x) + p0_x;
            *y = (t * s1_y) + p0_y;

            return true;
        }

        return false;
    }

    class BentleyOttmann {
    public:

        BentleyOttmann() = default;

        event_point_t create_event(const vec2_f & p0,
                                    const vec2_f & p1,
                                    event_type_t type, uint8_t
                                    precision = 0) {
            // convert to fixed precision
            vec2_32i p0_i = p0<<precision;
            vec2_32i p1_i = p1<<precision;

            return create_event(p0_i, p1_i, type);
        }

        event_point_t create_event(const vec2_32i & p0,
                                    const vec2_32i & p1,
                                    event_type_t type) {
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

        std::vector<vec2_32i> & compute(vec2_f * pts, int size, uint8_t precision) {

            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(pts[ix], pts[ix+1], event_type_t::START);
                event_point_t e2 = create_event(pts[ix], pts[ix+1], event_type_t::END);
                Queue.insert(e1);
                Queue.insert(e2);
            }

            return compute_internal();
        }

        std::vector<vec2_32i> & compute(vec2_32i * pts, int size) {

            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(pts[ix], pts[ix+1], event_type_t::START);
                event_point_t e2 = create_event(pts[ix], pts[ix+1], event_type_t::END);
                Queue.insert(e1);
                Queue.insert(e2);
            }

            return compute_internal();
        }

        std::vector<vec2_32i> & compute(segment_t * seg, int size) {

            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(seg[ix], event_type_t::START);
                event_point_t e2 = create_event(seg[ix], event_type_t::END);
                Queue.insert(e1);
                Queue.insert(e2);
            }

            return compute_internal();
        }

        std::vector<vec2_32i> & compute_internal() {
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

            return I;
        }

        void handleEventPoint(event_point_t & event) {
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
//            float x, y;
//            bool intersects = get_line_intersection(
//                    l.p0.x, l.p0.y, l.p1.x, l.p1.y,
//                    r.p0.x, r.p0.y, r.p1.x, r.p1.y,
//                    &x, &y
//            );

            rational_t x_r{}, y_r{};
            bool intersects = get_line_intersection_rational(
                    l.p0.x, l.p0.y, l.p1.x, l.p1.y,
                    r.p0.x, r.p0.y, r.p1.x, r.p1.y,
                    &x_r, &y_r
            );

            if(!intersects)
                return;

            // this is important for now, otherwise
            // we won't discard if it happened above us
            int x = x_r.toFixed();
            int y = y_r.toFixed();

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