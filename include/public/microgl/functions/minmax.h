#pragma once

namespace microgl {
    namespace functions {

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
        inline number max(const number * list, const unsigned size) {
            number max=*list;
            for(auto it = list; it!=list+size; it++)
                if(*it>max) max=*it;
            return max;
        }

        template<typename number>
        inline number min(const number * list, const unsigned size) {
            number min=*list;
            for(auto it = list; it!=list+size; it++)
                if(*it<min) min=*it;
            return min;
        }

        template<typename number>
        inline number abs_max(const number * list, const unsigned size) {
            number max=*list;
            if(max<0) max=-max;
            for(auto it = list; it!=list+size; it++) {
                auto val= (*it)<0?-(*it):(*it);
                if(val>max) max=val;
            }
            return max;
        }

        template<typename number>
        inline number abs_min(const number * list, const unsigned size) {
            number min=*list;
            if(min<0) min=-min;
            for(auto it = list; it!=list+size; it++) {
                auto val= (*it)<0?-(*it):(*it);
                if(val<min) min=val;
            }
            return min;
        }

    }
}
