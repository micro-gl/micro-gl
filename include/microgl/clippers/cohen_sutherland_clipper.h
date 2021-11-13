/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

namespace microgl {
    namespace clipping {
        template<typename number>
        class cohen_sutherland_clipper {
        private:
            using code = unsigned int;
            static const code INSIDE = 0b0000; // 0000
            static const code LEFT = 0b0001;   // 0001
            static const code RIGHT = 0b0010;  // 0010
            static const code BOTTOM = 0b0100; // 0100
            static const code TOP = 0b1000;    // 1000

        public:
            using const_ref = const number &;
            struct result {
                number x0, y0, x1, y1;
                number alpha_0, alpha_1;
                bool inside;
            };

            static auto compute(const_ref x0, const_ref y0, const_ref x1,
                                const_ref y1, const_ref left, const_ref top,
                                const_ref right, const_ref bottom) -> result {
                result r;
                r.alpha_0 = number(0);
                r.alpha_1 = number(1);
                r.x0 = x0;
                r.y0 = y0;
                r.x1 = x1;
                r.y1 = y1;
                r.inside = false;
                // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
                code out_code_0 = compute_out_code(x0, y0, left, top, right, bottom);
                code out_code_1 = compute_out_code(x1, y1, left, top, right, bottom);

                while (true) {
                    if (!(out_code_0 | out_code_1)) {
                        // bitwise OR is 0: both points inside window; trivially accept and exit loop
                        r.inside = true;
                        break;
                    } else if (out_code_0 & out_code_1) {
                        // bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
                        // or BOTTOM), so both must be outside window; exit loop (accept is false)
                        break;
                    } else {
                        // failed both tests, so calculate the line segment to clip
                        // from_sampler an outside point to an intersection with clip edge
                        number x, y;

                        // At least one endpoint is outside the clip rectangle; pick it.
                        code out_code_out = out_code_0 ? out_code_0 : out_code_1;

                        // Now find the intersection point;
                        // use formulas:
                        //   slope = (y1 - y0) / (x1 - x0)
                        //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
                        //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
                        // No need to worry about divide-by-zero because, in each case, the
                        // outcode bit being tested guarantees the denominator is non-zero
                        auto dx = r.x1 - r.x0;
                        auto dy = r.y1 - r.y0;
                        auto alpha = number(0);

                        if (out_code_out & BOTTOM) {           // point is below the clip window
                            alpha = (bottom - r.y0) / dy;
                            x = r.x0 + dx * alpha;
                            y = bottom;
                        } else if (out_code_out & TOP) { // point is above the clip window
                            alpha = (top - r.y0) / dy;
                            x = r.x0 + dx * alpha;
                            y = top;
                        } else if (out_code_out & RIGHT) {  // point is to the right of clip window
                            alpha = (right - r.x0) / dx;
                            y = r.y0 + dy * alpha;
                            x = right;
                        } else if (out_code_out & LEFT) {   // point is to the left of clip window
                            alpha = (left - r.x0) / dx;
                            y = r.y0 + dy * alpha;
                            x = left;
                        }

                        // Now we move outside point to intersection point to clip
                        // and get ready for next pass.
                        if (out_code_out == out_code_0) {
                            r.x0 = x;
                            r.y0 = y;
                            r.alpha_0 = alpha;
                            out_code_0 = compute_out_code(r.x0, r.y0, left, top, right, bottom);
                        } else {
                            r.x1 = x;
                            r.y1 = y;
                            r.alpha_1 = alpha;
                            out_code_1 = compute_out_code(r.x1, r.y1, left, top, right, bottom);
                        }
                    }
                }
                return r;
            }

        private:
            static code compute_out_code(const_ref x, const_ref y,
                                         const_ref left, const_ref top,
                                         const_ref right, const_ref bottom) {
                code code = INSIDE;     // initialised as being inside of [[clip window]]
                if (x < left)  code |= LEFT;            // to the left of clip window
                else if (x > right) code |= RIGHT;      // to the right of clip window
                if (y < top) code |= TOP;               // below the clip window
                else if (y > bottom) code |= BOTTOM;    // above the clip window
                return code;
            }
        };
    }
}