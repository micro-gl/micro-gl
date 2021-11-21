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

#include "vec2.h"
#include "math.h"

namespace microtess {

    using index = unsigned int;

    /**
     * Elliptic Arc divider
     * @tparam number the number type of a vertex
     * @tparam container_type the output container type
     */
    template<typename number, class container_type>
    class elliptic_arc_divider {
    public:
        using vertex = microtess::vec2<number>;

        elliptic_arc_divider()=delete;
        elliptic_arc_divider(const elliptic_arc_divider &)=delete;
        elliptic_arc_divider(elliptic_arc_divider &&)=delete;
        elliptic_arc_divider & operator=(const elliptic_arc_divider &)=delete;
        elliptic_arc_divider & operator=(elliptic_arc_divider &&)=delete;
        ~elliptic_arc_divider()=delete;

        static void compute(
                container_type &output,
                number center_x,
                number center_y,
                number radius_x,
                number radius_y,
                number rotation,
                number start_angle_rad,
                number end_angle_rad,
                unsigned divisions=32,
                bool anti_clockwise=false) {
            if(divisions<=0) return;
            const auto pi = microtess::math::pi<number>();
            const auto two_pi = pi * number(2);
            const auto half_pi = pi / number(2);
            const auto zero = number(0);
            while(start_angle_rad<zero) start_angle_rad+=two_pi;
            while(end_angle_rad<zero) end_angle_rad+=two_pi;
            auto delta = end_angle_rad - start_angle_rad;
            if (delta==0) return;
            const auto delta_abs = delta<0 ? -delta : delta;
            const bool full_circle_or_more= delta_abs >= two_pi;
            if(start_angle_rad<zero) start_angle_rad+=two_pi;
            if(end_angle_rad<zero) end_angle_rad+=two_pi;
            delta = end_angle_rad - start_angle_rad;
            if (!anti_clockwise) { // clockwise
                if (delta < zero) end_angle_rad += two_pi;
                else if(full_circle_or_more) end_angle_rad =start_angle_rad+two_pi;
            } else {
                if (delta > zero) end_angle_rad -= two_pi;
                else if(full_circle_or_more) end_angle_rad =start_angle_rad-two_pi;
            }
            delta = end_angle_rad - start_angle_rad;
            // we test_texture for greater in case of precision issues
            delta = delta / number(divisions-(full_circle_or_more?1:0));
            auto radians = start_angle_rad;
            auto rotation_sin = microtess::math::sin_cpu(rotation);
            auto rotation_cos = microtess::math::cos_cpu(rotation);
            index first_index= output.size(), last_index=first_index;
            number min_degree=start_angle_rad<=end_angle_rad?start_angle_rad:end_angle_rad;
            number max_degree=start_angle_rad>=end_angle_rad?start_angle_rad:end_angle_rad;
            for (index ix = 0; ix < divisions; ++ix) {
                number radians_clipped=radians;
                { // due to precision errors when adding radians they might overflow at the end
                    if(radians<min_degree) radians_clipped=min_degree;
                    if(radians>max_degree) radians_clipped=max_degree;
                }
                auto sine = microtess::math::sin_cpu(radians_clipped);
                auto cosine = microtess::math::sin_cpu(radians_clipped + half_pi);
                // currently , we assume pixel coords and radians have the same precision
                // this probably is not a very good idea
                auto x = cosine * radius_x;
                auto y = sine * radius_y;
                // now apply rotation
                auto rotated_x= x*rotation_cos - y*rotation_sin;
                auto rotated_y= x*rotation_sin + y*rotation_cos;
                const vertex point={(center_x + rotated_x), (center_y + rotated_y)};
                const bool skip=ix!=0 && point==output[last_index];
                if(!skip) {
                    output.push_back(point);
                    last_index=output.size()-1;
                }
                radians += delta;
            }
            // in addition to the clipping we perform, if it was a full circle, we need
            // to use first sample as last one
            if(full_circle_or_more) output[last_index]=output[first_index];
        }
    };
}
