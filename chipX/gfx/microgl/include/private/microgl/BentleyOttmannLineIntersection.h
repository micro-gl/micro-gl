#pragma once

#include <microgl/Types.h>
#include "AVLTree2.h"
#include <vector>
#include <iostream>
#include <stdexcept>

namespace tessellation {

//    http://www.normalesup.org/~cagne/internship_l3/mncubes_red/doc/html/bentley__ottmann_8cpp_source.html

#define max(a,b) (a)>(b) ? (a) : (b)
#define min(a,b) (a)<(b) ? (a) : (b)
//#define clamp(a,l,r) (min(max((a), (l)), (r)))
#define abs(a) ((a)<0 ? -(a) : (a))
#define E 1.0f

    enum class event_type_t {
        START, END, Intersection
    };

    struct rational_t {

//        mutable
        mutable long numerator = 0, denominator=1;
        int L = 0;

        rational_t() {};

        rational_t(long n) : rational_t(n, 1) {
        }

        rational_t(long n , long d) : numerator{n}, denominator{d} {
        }

        rational_t(const rational_t & val) {
            numerator = val.numerator;
            denominator = val.denominator;
        }

        rational_t & operator=(const rational_t & val) {
            if(this==&val)
                return *this;
            this->numerator = val.numerator;
            this->denominator = val.denominator;
            return *this;
        }

        rational_t & operator=(const long & val) {
            numerator = val;
            denominator = 1;
            return *this;
        }

        float toFloat() const {
            return float(numerator)/float(denominator);
        };

        bool isRegular() const {
            return denominator!=0;
        }

        bool isIrregular() const {
            return denominator!=0;
        }

        bool isUndefined() const {
            return numerator==0 && denominator==0;
        }

        bool isDefined() const {
            return !isUndefined();
        }

        bool isPositiveInfinity() const {
            return numerator>0 && denominator==0;
        }

        bool isNegativeInfinity() const {
            return numerator<0 && denominator==0;
        }

        int toFixed(uint8_t precision = 0) const {
            return (numerator<<precision)/(denominator);
        };

        static const rational_t maximum(const rational_t &lhs,
                                        const rational_t &rhs) {
            return (lhs<=rhs) ? rhs : lhs;
        }

        static const rational_t minimum(const rational_t &lhs,
                                        const rational_t &rhs) {
            return (lhs<=rhs) ? lhs : rhs;
        }

        static const rational_t clamp(const rational_t &a,
                                      const rational_t &left,
                                      const rational_t &right) {
            rational_t c = minimum(maximum(a, left), right);
            return c;
        }

        rational_t absolute() const {
            long n = numerator;
            long d = denominator;

            if(isNegative()) {
                n = numerator<0 ? -numerator : numerator;
                d = denominator<0 ? -denominator : denominator;
            } else {

            }

            return rational_t{n, d};
        }

        void makeDenominatorPositive() const {
            if(denominator < 0) {
                numerator = -numerator;
                denominator = -denominator;
            }

        }

        rational_t operator*(const long & val) const {
            long n = numerator*val;
            return rational_t{n, denominator};
        }

        rational_t operator*(const rational_t & val) const {
            throwIfDenomIsZero();

            long n = numerator*val.numerator;
            long d = denominator*val.denominator;
            return rational_t{n, d};
        }

        rational_t operator/(const long & val) const {
            return rational_t{numerator, denominator*val};
        }

        rational_t operator/(const rational_t & val) const {
            throwIfDenomIsZero();

            long n = numerator*val.denominator;
            long d = denominator*val.numerator;
            return rational_t{n, d};
        }

        rational_t operator+(const long & val) const {
            long n = numerator + denominator*val;
            return rational_t{n, denominator};
        }

        rational_t operator+(const rational_t & val) const {
            throwIfDenomIsZero();

            long n,d;
            if(denominator!=val.denominator) {
                n = numerator*val.denominator + val.numerator*denominator;
                d = denominator * val.denominator;
            } else {
                n = numerator + val.numerator;
                d = denominator;
            }
            return rational_t{n, d};
        }

        rational_t operator-(const long & val) const {
            long n = numerator - denominator*val;
            return rational_t{n, denominator};
        }

        rational_t operator-() const {
            return rational_t{0, 1} - *this;
        }

        rational_t operator-(const rational_t & val) const {
            throwIfDenomIsZero();
            long n,d;
            if(denominator!=val.denominator) {
                n = numerator*val.denominator - val.numerator*denominator;
                d = denominator * val.denominator;
            } else {
                n = numerator - val.numerator;
                d = denominator;
            }

            return rational_t{n, d};
        }

        void throwIfDenomIsZero() const {
            if(denominator==0)
                throw std::invalid_argument( "denom==0" );
        }

        // boolean operations here

        bool operator!=(const long & rhs) const {
            return !(*this==rhs);
        }

        bool operator==(const long & rhs) const {
            if(!isRegular())
                return false;

            if(rhs!=0)
                return numerator==rhs*denominator;

            return numerator==0;
        }

        bool operator==(const rational_t & rhs) const {
            if(!isRegular()) {
                if(isPositiveInfinity()==rhs.isPositiveInfinity())
                    return true;
                else if(isNegativeInfinity()==rhs.isNegativeInfinity())
                    return true;
            }

            return numerator*rhs.denominator==rhs.numerator*denominator;
        }

        bool operator!=(const rational_t & val) const {
            return !(*this==val);
        }

//        bool operator >= (const int & rhs) const {
//            return  !(*this<rhs);
//        }
//
//        bool operator > (const int & rhs) const {
//            return  !(*this<=rhs);
//        }
//
//        bool operator <= (const int & rhs) const {
//            return (*this<rhs) || (*this==rhs);
//        }

//        bool operator < (const int & val) const {
//            if(!isRegular()) {
//                if(isPositiveInfinity())
//                    return false;
//                else if(isNegativeInfinity())
//                    return true;
//            }
//
//            makeDenominatorPositive();
//            if(val!=0)
//                return numerator < denominator*val;
//            else {
//                return isNegative();
//            }
//        }

        bool operator <= (const rational_t & rhs) const {
            return (*this<rhs) || (*this==rhs);
        }

        bool operator >= (const rational_t & rhs) {
            return  !(*this<rhs);
        }

        bool operator < (const rational_t & rhs) const {
            if(!isRegular()) {
                if(isPositiveInfinity())
                    return false;
                else return rhs.isPositiveInfinity();
            }

            makeDenominatorPositive();
            if(rhs != 0)
                return numerator*rhs.denominator -
                       denominator*rhs.numerator < 0;
            else
                return isNegative();
        }

        bool operator > (const rational_t & rhs) const {
            return  !(*this<=rhs);
        }

        bool isPositive() const {
            throwIfDenomIsZero();
            return (numerator>=0 && denominator>=0)||
                   (numerator<=0 && denominator<=0);
        }

        bool isNegative() const {
            return !isPositive();
        }

    };

    typedef vec2<rational_t> vec2_rat;

    struct segment_t {
        vec2_32i p0, p1;
        vec2_32i latest_compare_point;

        segment_t() = default;

        segment_t(const segment_t & val) = default;

        segment_t(const vec2_32i & p0_, const vec2_32i & p1_) {
            p0 = p0_;
            p1 = p1_;
        }

        bool operator==(const segment_t & rhs) const {
            return p0==rhs.p0 && p1==rhs.p1;
        }

        rational_t compute_slope() const {

            return rational_t(p1.x - p0.x)/rational_t(p1.y - p0.y);
        }

        // special intersection with the ray encapsulating the segment
        rational_t compute_x_intersection_with(const vec2_32i & p) const {
            // we presume we already have intersection with p.y
            rational_t slope = compute_slope();//{dx, dy};
            auto dy = slope.denominator;
            rational_t x2{10,10};

            // dy==0
            // infinite slope
            if (dy==0) {
                x2 = rational_t::clamp(p.x, p0.x, p1.x);
            } else {

                x2 = rational_t{p0.x} + rational_t(p.y - p0.y)*slope;
//
//                float slope_f = float(slope.numerator) / float(slope.denominator);
//                float x2_f = float(p0.x) + float(p.y - p0.y)*slope_f;
//
//                x2 = x2_f;

            }

            return {x2};
        }

        bool contains(const vec2_32i & p) {

            // this is an optimization to avoid divisions
            rational_t slope = compute_slope();//{dx, dy};
            auto dy = slope.denominator;

            bool lies_on_ray;

            if(dy!=0) {

                rational_t x_1 = compute_x_intersection_with({p.x, p.y});
                rational_t x_2 = compute_x_intersection_with({p.x, p.y + 1});

                x_1.makeDenominatorPositive();
                x_2.makeDenominatorPositive();

//                const rational_t & min_1 = rational_t::minimum(x_1, x_2);
//                const rational_t & max_1 = rational_t::maximum(x_1, x_2);
//                const rational_t & min_2 = {p.x, 1};
//                const rational_t & max_2 = {p.x, 1};
//
                const auto & min_1 = rational_t::minimum(x_1, x_2).toFixed();
                const auto & max_1 = rational_t::maximum(x_1, x_2).toFixed();
                const auto & min_2 = rational_t{p.x, 1}.toFixed();
                const auto & max_2 = rational_t{p.x, 1}.toFixed();

                bool lines_intersect_on_p = (min_2 - max_1)<=1 && (min_1-max_2)<=1;
                lies_on_ray = lines_intersect_on_p;

            } else {
                lies_on_ray = p0.y==p.y;
            }

            // check edge cases beyond the segment
            if(lies_on_ray) {
                auto minX = min(p0.x, p1.x);
                auto minY = min(p0.y, p1.y);
                auto maxX = max(p0.x, p1.x);
                auto maxY = max(p0.y, p1.y);

                bool outside_x = minX > p.x || maxX < p.x;
                bool outside_y = minY > p.y || maxY < p.y;

                if(outside_x || outside_y)
                    return false;
            }

            return lies_on_ray;
        }

        bool isStart(const vec2_32i & p) {
            return p0==p;
        }

        bool isEnd(const vec2_32i & p) {
            return p1==p;
        }

    };

    struct segment_order
    {

        long cmp(const segment_t& lhs, const segment_t& rhs) {

            rational_t lhs_x = lhs.compute_x_intersection_with(p);
            rational_t rhs_x = rhs.compute_x_intersection_with(p);
            rational_t lhs_x_at_1 = lhs.compute_x_intersection_with({p.x, p.y + 1});
            rational_t rhs_x_at_1 = rhs.compute_x_intersection_with({p.x, p.y + 1});

            lhs_x.makeDenominatorPositive();
            rhs_x.makeDenominatorPositive();
            lhs_x_at_1.makeDenominatorPositive();
            rhs_x_at_1.makeDenominatorPositive();

            // let's test if rhs passes through the unit cube which has
            // top-left at (lhs_x, p.y) ?

//            const rational_t & min_1 = rational_t::minimum(lhs_x, lhs_x_at_1);
//            const rational_t & max_1 = rational_t::maximum(lhs_x, lhs_x_at_1);
//            const rational_t & min_2 = rational_t::minimum(rhs_x, rhs_x_at_1);
//            const rational_t & max_2 = rational_t::maximum(rhs_x, rhs_x_at_1);

            const auto & min_1 = rational_t::minimum(lhs_x, lhs_x_at_1).toFixed();
            const auto & max_1 = rational_t::maximum(lhs_x, lhs_x_at_1).toFixed();
            const auto & min_2 = rational_t::minimum(rhs_x, rhs_x_at_1).toFixed();
            const auto & max_2 = rational_t::maximum(rhs_x, rhs_x_at_1).toFixed();

            bool lines_intersect_on_p = (min_2 - max_1)<=1 && (min_1 - max_2)<=1;

            bool inside = lines_intersect_on_p;

            if(!inside) {
                rational_t compare = (lhs_x - rhs_x);
                compare.makeDenominatorPositive();
                return compare.numerator;
            }

            bool lhs_before = max_1 <= min_2;
            bool rhs_before = max_2 <= min_1;

//            if(lhs_before)
//                return -1;
//
//            if(rhs_before)
//                return 1;

            // if they are equal at p, we need to break tie with slope
            rational_t lhs_slope = lhs.compute_slope();
            rational_t rhs_slope = rhs.compute_slope();

            // return

            lhs_slope.makeDenominatorPositive();
            rhs_slope.makeDenominatorPositive();

            bool lhs_has_infinite_slope = lhs_slope.denominator==0;
            bool rhs_has_infinite_slope = rhs_slope.denominator==0;

            if(lhs_has_infinite_slope)
                return 1;

            if(rhs_has_infinite_slope)
                return -1;

            auto slope_compare = lhs_slope - rhs_slope;

            return slope_compare.numerator;
        }

        bool isPreceding(const segment_t& lhs, const segment_t& rhs)
        {
            return cmp(lhs, rhs) < 0;
        }

        void updateComparePoint(const vec2_32i & val) {
            p = val;
        }

        const vec2_32i & getComparePoint() {
            return p;
        }
    private:

        vec2_32i p;
    };

    struct event_point_t {
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

        rational_t s{1,1}, t{1,1};
        s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y));
        s.denominator = (-s2_x * s1_y + s1_x * s2_y);

        t.numerator = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x));
        t.denominator = (-s2_x * s1_y + s1_x * s2_y);

        if (s.isRegular() && t.isRegular() && s >= 0 && s <= 1 && t >= 0 && t <= 1)
        {
            *x = (t * s1_x) + p0_x;
            *y = (t * s1_y) + p0_y;

            return true;
        }

        return false;
    }

    bool get_line_intersection_rational_2(const vec2_rat& p0, const vec2_rat &p1,
                                          const vec2_rat &p2, const vec2_rat & p3,
                                          vec2_rat & intersection)
    {

        vec2_rat s1 = p1 - p0;
        vec2_rat s2 = p3 - p2;

        rational_t d = -s2.x * s1.y + s1.x * s2.y;
        rational_t s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / d;
        rational_t t = ( s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / d;

        if (s.isRegular() && t.isRegular() && s >= 0 && s <= 1 && t >= 0 && t <= 1) {
            intersection.x = (t * s1.x) + p0.x;
            intersection.y = (t * s1.y) + p0.y;

            // try this later
            //intersection = p0 + s1 * t;

            return true;
        }

        return false;
    }

    class BentleyOttmann {
    public:

        BentleyOttmann() = default;

        event_point_t create_event(const vec2_f & p0,
                                   const vec2_f & p1,
                                   event_type_t type,
                                   uint8_t precision = 4) {
            // convert to fixed precision
            vec2_32i p0_i, p1_i;

            // applying precision here
            p0_i = p0<<precision;
            p1_i = p1<<precision;

            return create_event(p0_i, p1_i, type, 0);
        }

        event_point_t create_event(const vec2_32i & p0,
                                   const vec2_32i & p1,
                                   event_type_t type,
                                   uint8_t precision = 4) {
            event_point_t pt;

            pt.segment.p0 = p0<<precision;
            pt.segment.p1 = p1<<precision;
            // todo:: this is important, reversing direction
//            pt.segment.p0 = p0.y<=p1.y ? p0 : p1;
//            pt.segment.p1 = p0.y<=p1.y ? p1 : p0;
            pt.type = type;

            return pt;
        }

        event_point_t create_event(const segment_t & segment,
                                   event_type_t type,
                                   uint8_t precision = 4) {
            return create_event(segment.p0, segment.p1,
                    type, precision);
        }

        event_point_t create_event(const vec2_rat & p0,
                                   const vec2_rat & p1,
                                   event_type_t type,
                                   uint8_t precision = 4) {
            event_point_t pt;

            pt.segment.p0 = {p0.x.toFixed(precision), p0.y.toFixed(precision)};
            pt.segment.p1 = {p1.x.toFixed(precision), p1.y.toFixed(precision)};
            // todo:: this is important, reversing direction
//            pt.segment.p0 = p0.y<=p1.y ? p0 : p1;
//            pt.segment.p1 = p0.y<=p1.y ? p1 : p0;
            pt.type = type;

            return pt;
        }

        std::vector<vec2_32i> & compute(vec2_f * pts,
                                        int size,
                                        uint8_t precision) {

            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(pts[ix], pts[ix+1],
                        event_type_t::START, precision);
                event_point_t e2 = create_event(pts[ix], pts[ix+1],
                        event_type_t::END, precision);
                Queue.insert(e1);
                Queue.insert(e2);
            }

            return compute_internal();
        }

        std::vector<vec2_32i> & compute(vec2_32i * pts, int size, uint8_t precision) {

            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(pts[ix], pts[ix+1],
                        event_type_t::START, precision);
                event_point_t e2 = create_event(pts[ix], pts[ix+1],
                        event_type_t::END, precision);
                Queue.insert(e1);
                Queue.insert(e2);
            }

            return compute_internal();
        }

        std::vector<vec2_32i> & compute(segment_t * seg, int size, uint8_t precision) {

            for (int ix = 0; ix < size; ++ix) {
                event_point_t e1 = create_event(seg[ix],
                        event_type_t::START, precision);
                event_point_t e2 = create_event(seg[ix],
                        event_type_t::END, precision);
                Queue.insert(e1);
                Queue.insert(e2);
            }

            return compute_internal();
        }

        std::vector<vec2_32i> & compute_internal() {
            rational_t zero{0, 0};

            S.getComparator().updateComparePoint({0,0});

            while (!Queue.isEmpty()) {
                event_point_t event = Queue.removeMinKey();

                /*
                // do this later to handle multiple
                while (Queue.contains(event)) {
                    event_point_t event2 = Queue.removeMinKey();
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
            if(event.type==event_type_t::START) {
                U_p.push_back(event.segment);

//                while (Queue.contains(event)) {
//                    event_point_t event2 = Queue.removeMinKey();
//                    U_p.push_back(event2.segment);
//                }
            }

            auto & p = event.getPoint();
            auto p_last = S.getComparator().getComparePoint();
            bool has_last_compare_point = p_last.y!=0;
            if(!has_last_compare_point) {
                p_last = p;
            }

            // update the scanning y
            S.getComparator().updateComparePoint(p);

            //
            // 2. find all segments that contain p, and classify them
            //
            std::vector<segment_t> L_p, C_p;
            // create a zero length segment at p
            segment_t p_segment{p, p};
            Status::Node * node = S.findLowerBoundOf(p_segment);

            bool seq_started=false;
            while(node!=nullptr) {
//                std::cout << "check 1" <<std::endl;

                segment_t & tested_segment = node->key;//.contains(p)
                bool tested_segment_contains_p =
                        tested_segment.contains(p);

                if(tested_segment_contains_p) {
                    seq_started=true;
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
                    if(seq_started)
                        break;
                }

                node = S.predecessor(node);
            }

            // end classification

            // 3. test intersection and report

            bool intersection_found = (U_p.size() + L_p.size() + C_p.size()) > 1;

            if(intersection_found) {
                // report intersection
                std::cout << "report intersection with p : " + std::to_string(p.x) +
                             ", " + std::to_string(p.y) << std::endl;

                I.push_back({p.x, p.y});
            }

            // 5. delete segments in L_p, C_p from Status
//            S.getComparator().updateComparePoint(p_last);

            for (auto & ix : C_p) {
                S.getComparator().updateComparePoint(ix.latest_compare_point);
                S.remove(ix);
            }

            for (auto & ix : L_p) {
                S.getComparator().updateComparePoint(ix.latest_compare_point);
                S.remove(ix);
            }

            S.getComparator().updateComparePoint(p);
            // 6. insert U_p and C_p
            for (auto & ix : U_p) {
                ix.latest_compare_point = p;
                S.insert(ix);
            }

            for (auto & ix : C_p) {
                ix.latest_compare_point = p;
                S.insert(ix);
            }

            // 8
            bool is_Up_and_C_p_empty = U_p.empty() && C_p.empty();

            if(is_Up_and_C_p_empty) {
                // create a zero length segment at p
                // this requires more thinking
                segment_t p_segment2{p, p};
                S.getComparator().updateComparePoint(p);
                Status::Node * left = S.findLowerBoundOf(p_segment2);

                if(left) {
                    Status::Node *right = S.successor(left);

                    if (right) {
                        find_new_event(left->key, right->key, p);
                    }

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

                if(right_neighbor!= nullptr) {
                    bool s1 = min_node->key.p0==min.p0 && min_node->key.p1==min.p1;
                    bool s2 = max_node->key.p0==max.p0 && max_node->key.p1==max.p1;

                    if(!s1 || !s2) {
                        int a =5;
                    }

                    find_new_event(max, right_neighbor->key, p);
                }

            }


        }

        void find_new_event(const segment_t & l,
                            const segment_t & r,
                            const vec2_32i & p) {
            if(l==r)
                return;

            float x_f, y_f;
            bool intersects_f = get_line_intersection(
                    l.p0.x, l.p0.y, l.p1.x, l.p1.y,
                    r.p0.x, r.p0.y, r.p1.x, r.p1.y,
                    &x_f, &y_f
            );
//
//            rational_t x_r{}, y_r{};
//            bool intersects = get_line_intersection_rational(
//                    l.p0.x, l.p0.y, l.p1.x, l.p1.y,
//                    r.p0.x, r.p0.y, r.p1.x, r.p1.y,
//                    &x_r, &y_r
//            );

//            vec2_rat intersection;
//            bool intersects = get_line_intersection_rational_2(
//                    l.p0, l.p1, r.p0, r.p1,
//                    intersection
//            );
//
//            bool isValid = intersects &&
//                           intersection.x.isRegular() &&
//                           intersection.y.isRegular();
//
            bool isValid = intersects_f;
            if(!isValid)
                return;

            // this is important for now, otherwise
            // we won't discard if it happened above us
            vec2_32i intersection;

            intersection.x = x_f;
            intersection.y = y_f;

            bool below_p_line = intersection.y > p.y;
            bool on_p_and_right = intersection.y==p.y && intersection.x > p.x;

            if(below_p_line || on_p_and_right) {
                event_point_t event;
                event.segment.p0 = intersection;
                event.segment.p1 = intersection;
                event.type = event_type_t::Intersection;

                if(Queue.search(event))
                    return;

                Queue.insert(event);

                std::cout << "found intersection : " + std::to_string(intersection.x) +
                             ", " + std::to_string(intersection.y) << std::endl;

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