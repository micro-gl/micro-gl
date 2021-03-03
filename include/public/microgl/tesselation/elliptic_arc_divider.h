#pragma once

#include <microgl/vec2.h>
#include <microgl/dynamic_array.h>
#include <microgl/math.h>

namespace microgl {

    namespace tessellation {

        using index = unsigned int;
        using precision_t = unsigned char;
        using math = microgl::math;
        using namespace microgl;

        template <typename number>
        class elliptic_arc_divider {
        public:
            using vertex = vec2<number>;

            static void compute(
                    dynamic_array<vertex> &output,
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
                const auto two_pi = math::pi<number>() * number(2);
                const auto half_pi = math::pi<number>() / number(2);
                const auto zero = number(0);
                auto delta = end_angle_rad - start_angle_rad;
                if (delta==0) return;
                const bool full_circle_or_more= microgl::math::abs(delta) >= two_pi;
                start_angle_rad = math::mod(start_angle_rad, two_pi);
                end_angle_rad = math::mod(end_angle_rad, two_pi);
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
                delta = delta / number(divisions);
                auto radians = start_angle_rad;
                auto rotation_sin = math::sin(rotation);
                auto rotation_cos = math::cos(rotation);
                index first_index= output.size(), last_index=first_index;
                number min_degree=start_angle_rad<=end_angle_rad?start_angle_rad:end_angle_rad;
                number max_degree=start_angle_rad>=end_angle_rad?start_angle_rad:end_angle_rad;
                for (index ix = 0; ix <= divisions; ++ix) {
                    number radians_clipped=radians;
                    { // due to precision errors when adding radians they might overflow at the end
                        if(radians<min_degree)
                            radians_clipped=min_degree;
                        if(radians>max_degree)
                            radians_clipped=max_degree;
                    }
                    auto sine = math::sin(radians_clipped);
                    auto cosine = math::sin(radians_clipped + half_pi);
                    // currently , we assume pixel coords and radians have the same precision
                    // this probably is not a very good idea
                    auto x = cosine * radius_x;
                    auto y = sine * radius_y;
                    // now apply rotation
                    auto rotated_x= x*rotation_cos - y*rotation_sin;
                    auto rotated_y= x*rotation_sin + y*rotation_cos;
                    const vertex point={(center_x + rotated_x), (center_y + rotated_y)};
                    const bool skip=ix!=0 && point==output[last_index];
                    if(!skip) last_index=output.push_back(point);
                    radians += delta;
                }
                // in addition to the clipping we perform, if it was a full circle, we need
                // to use first sample as last one
                if(full_circle_or_more) output[last_index]=output[first_index];
            }

        private:
        };


    }

}
