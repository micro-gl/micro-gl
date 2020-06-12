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

        template<typename T>
        inline T min(const T &p0, const T &p1) {
            return p0 < p1 ? p0 : p1;
        }
        template<>
        inline float min(const float &p0, const float &p1) {
            return fmin(p0, p1);
        }

        template<typename T>
        inline T min(const T &p0, const T &p1, const T &p2) {
            return min(min(p0, p1), p2);
        }

        template<typename T>
        inline T min(const T &p0, const T &p1, const T &p2, const T &p3) {
            return min(min(min(p0, p1), p2), p3);
        }

        template<typename T>
        inline T min(const T &p0, const T &p1, const T &p2, const T &p3, const T &p4) {
            return min(min(min(min(p0, p1), p2), p3), p4);
        }

        template<typename T>
        inline T max(const T &p0, const T &p1) {
            return p0 > p1 ? p0 : p1;
        }
        template<>
        inline float max(const float &p0, const float &p1) {
            return fmax(p0, p1);
        }

        template<typename T>
        inline T max(const T &p0, const T &p1, const T &p2) {
            return max(max(p0, p1), p2);
        }

        template<typename T>
        inline T max(const T &p0, const T &p1, const T &p2, const T &p3) {
            return max(max(max(p0, p1), p2), p3);
        }

        template<typename T>
        inline T max(const T &p0, const T &p1, const T &p2, const T &p3, const T &p4) {
            return max(max(max(max(p0, p1), p2), p3), p4);
        }

//        template<typename T>
//        inline T clamp(const T &val, const T &e0, const T &e1) {
//            return min(max(val, e0), e1);
//        }
    }
}
