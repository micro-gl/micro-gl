#pragma once
#include <cmath>

namespace microgl {
    namespace functions {

#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))
#define min3(a, b, c) (min_(min_(a,b), c))
#define max3(a, b, c) (max_(max_(a,b), c))
#define clamp_(v, e0, e1) (min_(max_(v,e0),e1))
#define abs_(a) ((a)<0 ? -(a) : (a))

        template<typename number>
        inline number min(const number &p0, const number &p1) {
            return p0 < p1 ? p0 : p1;
        }
        template<>
        inline float min<float>(const float &p0, const float &p1) {
            return fmin(p0, p1);
        }

        template<typename number>
        inline number min(const number &p0, const number &p1, const number &p2) {
            return min(min(p0, p1), p2);
        }

        template<typename number>
        inline number min(const number &p0, const number &p1, const number &p2, const number &p3) {
            return min(min(min(p0, p1), p2), p3);
        }

        template<typename number>
        inline number min(const number &p0, const number &p1, const number &p2, const number &p3, const number &p4) {
            return min(min(min(min(p0, p1), p2), p3), p4);
        }

        template<typename number>
        inline number min(const number &p0, const number &p1, const number &p2, const number &p3, const number &p4, const number &p5) {
            return min(min(min(min(min(p0, p1), p2), p3), p4), p5);
        }

        template<typename number>
        inline number min(const std::initializer_list<number> & list) {
            number min=*list.begin();
            for(auto it = list.begin(); it!=list.end(); it++) {
                if(*it<min) min=*it;
            }
            return min;
        }

        template<typename number>
        inline number max(const number &p0, const number &p1) {
            return p0 > p1 ? p0 : p1;
        }
        template<>
        inline float max(const float &p0, const float &p1) {
            return fmax(p0, p1);
        }

        template<typename number>
        inline number max(const number &p0, const number &p1, const number &p2) {
            return max(max(p0, p1), p2);
        }

        template<typename number>
        inline number max(const number &p0, const number &p1, const number &p2, const number &p3) {
            return max(max(max(p0, p1), p2), p3);
        }

        template<typename number>
        inline number max(const number &p0, const number &p1, const number &p2, const number &p3, const number &p4) {
            return max(max(max(max(p0, p1), p2), p3), p4);
        }

        template<typename number>
        inline number max(const number &p0, const number &p1, const number &p2, const number &p3, const number &p4, const number &p5) {
            return max(max(max(max(max(p0, p1), p2), p3), p4), p5);
        }

        template<typename number>
        inline number max(const std::initializer_list<number> & list) {
            number max=*list.begin();
            for(auto it = list.begin(); it!=list.end(); it++) {
                if(*it>max) max=*it;
            }
            return max;
        }

    }
}
