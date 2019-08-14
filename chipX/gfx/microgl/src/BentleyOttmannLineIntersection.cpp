#include <microgl/BentleyOttmannLineIntersection.h>

namespace tessellation {

    BentleyOttmann::BentleyOttmann(bool DEBUG) : _DEBUG{DEBUG} {}

    event_point_t BentleyOttmann::create_event(const vec2_f &p0, const vec2_f &p1, event_type_t type, uint8_t precision) {
        // convert to fixed precision
        vec2_32i p0_i, p1_i;

        // applying precision here
        p0_i = p0<<precision;
        p1_i = p1<<precision;

        return create_event(p0_i, p1_i, type, 0);
    }

    event_point_t
    BentleyOttmann::create_event(const vec2_32i &p0, const vec2_32i &p1, event_type_t type, uint8_t precision) {
        event_point_t pt;

        pt.segment.p0 = p0<<precision;
        pt.segment.p1 = p1<<precision;
        pt.type = type;

        bool reverseY = pt.segment.p1.y<pt.segment.p0.y;

        if(reverseY) {
            std::swap(pt.segment.p0,pt.segment.p1);
            pt.type = pt.type==event_type_t::START ? event_type_t::END : event_type_t::START;
        }

        return pt;
    }

    event_point_t BentleyOttmann::create_event(const Segment &segment, event_type_t type, uint8_t precision) {
        return create_event(segment.p0, segment.p1,
                            type, precision);
    }

    event_point_t
    BentleyOttmann::create_event(const vec2_rat &p0, const vec2_rat &p1, event_type_t type, uint8_t precision) {
        event_point_t pt;

        pt.segment.p0 = {p0.x.toFixed(precision), p0.y.toFixed(precision)};
        pt.segment.p1 = {p1.x.toFixed(precision), p1.y.toFixed(precision)};
        pt.type = type;

        bool reverse = pt.segment.p1.y<pt.segment.p0.y;

        if(reverse) {
            std::swap(pt.segment.p0,pt.segment.p1);
            pt.type = type==event_type_t::START ? event_type_t::END : event_type_t::START;
        }

        return pt;
    }

    std::vector<vec2_32i> &BentleyOttmann::compute(vec2_f *pts, int size, uint8_t precision) {

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

    std::vector<vec2_32i> &BentleyOttmann::compute(vec2_32i *pts, int size, uint8_t precision) {

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

    std::vector<vec2_32i> &BentleyOttmann::compute(Segment *seg, int size, uint8_t precision) {
        PR = precision;
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

    void BentleyOttmann::find_new_event(const Segment &l, const Segment &r, const vec2_32i &p) {
        if(l==r)
            return;

        /*
        vec2_f intersection_internal_f;
        bool intersects_f = l.intersect(r,
                                        intersection_internal_f
        );
         */

        vec2_rat intersection_internal;
        bool intersects = l.intersect(r,
                                      intersection_internal
        );
//
        bool isValid = intersects &&
                       intersection_internal.x.isRegular() &&
                       intersection_internal.y.isRegular();

        if(!isValid)
            return;

        // this is important for now, otherwise
        // we won't discard if it happened above us
        vec2_32i intersection;

        intersection.x = intersection_internal.x.toFixed();
        intersection.y = intersection_internal.y.toFixed();

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

            if(_DEBUG)
                std::cout << "found intersection : " + std::to_string(intersection.x) +
                             ", " + std::to_string(intersection.y) << std::endl;

        }

    }

    std::vector<vec2_32i> &BentleyOttmann::compute_internal() {
        Rational zero{0, 0};

        S.getComparator().updateComparePoint({0,-10});

        while (!Queue.isEmpty()) {
            event_point_t event = Queue.removeMinKey();

            if(_DEBUG) {
                std::string info = std::to_string(event.getPoint().x) +
                                   "x" + std::to_string(event.getPoint().y);
                std::cout<<"event p " << info << std::endl;
            }

            handleEventPoint(event);
        }

        if(!S.isEmpty())
            _has_failed_for_sure = true;

        if(_has_failed_for_sure && _DEBUG) {
            std::cout << "Algorithm probaly has Failed" << std::endl;
        }

        destroy();

        return I;
    }

    void BentleyOttmann::handleEventPoint(event_point_t &event) {
        std::vector<Segment> U_p {};

        // for now suppose U(p) is always a singleton,
        // I will solve it later
        if(event.type==event_type_t::START) {
            U_p.push_back(event.segment);

        }

        std::vector<event_point_t> coincides {};
        // remove all same points
        while (!Queue.isEmpty() && Queue.findMin()->key.getPoint()==event.getPoint()) {
            event_point_t event2 = Queue.removeMinKey();
            coincides.push_back(event2);
        }

        // take upper points
        for(auto & c : coincides) {
            if(c.type==event_type_t::START)
                U_p.push_back(c.segment);
        }

        auto p = event.getPoint();

        // update the scanning y
        S.getComparator().updateComparePoint(p);
        S.getComparator().compareOnTheLine(false);

        //
        // 2. find all segments that contain p, and classify them
        //
        std::vector<Segment> L_p, C_p;
        // create a zero length segment at p
        Segment p_segment{p, p};
        Status::Node * node = S.findLowerBoundOf(p_segment);

        bool seq_started=false;
        while(node!=nullptr) {

            Segment & tested_segment = node->key;//.contains(p)
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
            if(_DEBUG)
                std::cout << "report intersection with p : " + std::to_string(p.x) +
                             ", " + std::to_string(p.y) << std::endl;

            I.push_back({p.x, p.y});
        }

        // 5. delete segments in L_p, C_p from Status
        S.getComparator().compareOnTheLine(true);

        for (auto & ix : C_p) {
            S.remove(ix);
        }

//        S.getComparator().compareOnTheLine(false);
//
//        for (auto & ix : C_p) {
//            S.remove(ix);
//        }


        S.getComparator().compareOnTheLine(true);
        for (auto & ix : L_p) {
            S.remove(ix);
        }
//        S.getComparator().compareOnTheLine(false);
//        for (auto & ix : L_p) {
//            S.remove(ix);
//        }

        // insert

        S.getComparator().updateComparePoint(p);
        S.getComparator().compareOnTheLine(false);

        // 6. insert U_p and C_p
        for (auto & ix : U_p) {
            S.insert(ix);
        }

        for (auto & ix : C_p) {
            S.insert(ix);
        }

        S.getComparator().compareOnTheLine(false);

        // 8
        bool is_Up_and_C_p_empty = U_p.empty() && C_p.empty();

        if(is_Up_and_C_p_empty) {
            // create a zero length segment at p
            // this requires more thinking
            Segment p_segment2{p, p};
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

            Segment min{}, max{};

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

            Status::Node * min_node = S.searchExact(min);
            Status::Node * max_node = S.searchExact(max);
            Status::Node * left_neighbor = S.predecessor(min_node);
            Status::Node * right_neighbor = S.successor(max_node);

            if(left_neighbor!= nullptr)
                find_new_event(left_neighbor->key, min, p);

            if(right_neighbor!= nullptr)
                find_new_event(max, right_neighbor->key, p);

            return;

        }


    }

    void BentleyOttmann::destroy() {
        Queue.clear();
        S.clear();
    }

    bool BentleyOttmann::hasTheAlgorithmFailed() {
        return _has_failed_for_sure;
    }

    std::vector<vec2_32i> &BentleyOttmann::getIntersections() {
        return I;
    }

    long segment_order::cmp(const Segment &lhs, const Segment &rhs) {

        Rational lhs_x = lhs.compute_x_intersection_with(p);
        Rational rhs_x = rhs.compute_x_intersection_with(p);
        Rational lhs_x_at_1 = lhs.compute_x_intersection_with({p.x, p.y + 1});
        Rational rhs_x_at_1 = rhs.compute_x_intersection_with({p.x, p.y + 1});

        lhs_x.makeDenominatorPositive();
        rhs_x.makeDenominatorPositive();
        lhs_x_at_1.makeDenominatorPositive();
        rhs_x_at_1.makeDenominatorPositive();

        // let's test if rhs passes through the unit cube which has
        // top-left at (lhs_x, p.y) ?

        const auto & min_1 = Rational::minimum(lhs_x, lhs_x_at_1).toFixed();
        const auto & max_1 = Rational::maximum(lhs_x, lhs_x_at_1).toFixed();
        const auto & min_2 = Rational::minimum(rhs_x, rhs_x_at_1).toFixed();
        const auto & max_2 = Rational::maximum(rhs_x, rhs_x_at_1).toFixed();

        bool lines_intersect_on_p = (min_2 - max_1)<=EE && (min_1 - max_2)<=EE;

        // may use it in the future
        bool x_ref_in_1 = (p.x - max_1)<=EE && (min_1 - p.x)<=EE;//(p.x>=min_1 && p.x<=max_1) || (p.x+1>=min_1 && p.x+1<=max_1);
        bool x_ref_in_2 = (min_2 - p.x)<=EE && (p.x - max_2)<=EE;//(p.x>=min_2 && p.x<=max_2) || (p.x+1>=min_2 && p.x+1<=max_2);

        bool inside = lines_intersect_on_p;// && x_ref_in_1 && x_ref_in_2;

        if(!inside) {
            Rational compare = (lhs_x - rhs_x);
            compare.makeDenominatorPositive();
            return compare.numerator;
        }

        // if they are equal at p, we need to break tie with slope
        Rational lhs_slope = lhs.compute_slope();
        Rational rhs_slope = rhs.compute_slope();

        // return

        lhs_slope.makeDenominatorPositive();
        rhs_slope.makeDenominatorPositive();

        long tie = 0;

        // I want to clarify that we never deal with negative infinity slopes
        if((lhs_slope.isPositiveInfinity() && rhs_slope.isPositiveInfinity()) ||
           (lhs_slope.isNegativeInfinity() && rhs_slope.isNegativeInfinity())
                )
            return 0;

        if(lhs_slope.isPositiveInfinity())
            return 1;
        else if(rhs_slope.isNegativeInfinity())
            return 1;
        else if(rhs_slope.isPositiveInfinity())
            return -1;
        else if(lhs_slope.isNegativeInfinity())
            return -1;
        else {
            tie = (lhs_slope - rhs_slope).numerator;
        }

        return isCompareOnTheLine() ? -tie : tie;
    }
}