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

#include <microgl/samplers/line_linear_gradient.h>

namespace microgl {
    namespace sampling {

        /**
         * given an angle, compute the gradient line in the [0,1]x[0,1] box.
         */
        template <typename number, unsigned N=10, typename rgba_=rgba_t<8,8,8,0>,
                enum precision $precision=precision::medium>
        class angular_linear_gradient : public line_linear_gradient<number, N, rgba_, $precision> {
            using base= line_linear_gradient<number, N, rgba_, $precision>;
            using point= vertex2<number>;

            point intersect(const point &a, const point &b,
                            const point &c, const point &d) const {
                const point s1 = b-a; // ab
                const point s2 = d-c; // cd
                const point dc = a-c; // ca
                const number denom = -s2.x * s1.y + s1.x * s2.y;
                //const number s = (-s1.y * dc.x + s1.x * dc.y) / denom; // (-ab.y*ca.x + ab.x*ca.y)
                const number t = ( s2.x * dc.y - s2.y * dc.x) / denom;
                return a + s1*t;
            }

        public:
            angular_linear_gradient() : base{} {}
            explicit angular_linear_gradient(const number & angle_degrees) : angular_linear_gradient() {
                setAngle(angle_degrees);
            };

            void setAngle(number angle_degrees) {
                angle_degrees = microgl::math::mod(angle_degrees, number(360));
                if(angle_degrees<0) angle_degrees+=number(360);
                const point left_bottom= {0, 0}, left_top= {0, 1};
                const point right_top= {1, 1}, right_bottom= {1, 0};
                const auto radians= microgl::math::deg_to_rad(angle_degrees);
                // center in unit box
                const auto center= point(1, 1)/2;
                const auto dir= point{microgl::math::cos(radians), microgl::math::sin(radians)};
//                const auto dir= point{1,0};
                // pre modified gradient line
                const auto p1= center;
                const auto p2= center + dir;
                const auto normal= point{-dir.y, dir.x};
                // infer correct endpoint for perpendicular lines intersection,
                // these will help us calculate the correct gradient line endpoints
                point a1, a2, b1, b2;
                if(angle_degrees>=0 && angle_degrees<90) {
                    a1= left_bottom; a2= a1+normal;
                    b1= right_top; b2= b1+normal;
                }
                else if(angle_degrees>=90 && angle_degrees<180) {
                    a1= right_bottom; a2= a1+normal;
                    b1= left_top; b2= b1+normal;
                }
                else if(angle_degrees>=180 && angle_degrees<270) {
                    a1= right_top; a2= a1+normal;
                    b1= left_bottom; b2= b1+normal;
                }
                else if(angle_degrees>=270 && angle_degrees<360) {
                    a1= left_top; a2= a1+normal;
                    b1= right_bottom; b2= b1+normal;
                }
                const auto start = intersect(p1, p2, a1, a2);
                const auto end = intersect(p1, p2, b1, b2);
                this->setNewLine(start, end);
            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                base::sample(u, v, bits, output);
            }

        private:

        };

    }
}