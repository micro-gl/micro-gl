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
                const auto two_pi = math::pi<number>() * number(2);
                const auto half_pi = math::pi<number>() / number(2);
                const auto zero = number(0);
                number epsilon = number(1) / number(100);
                if (microgl::math::abs(start_angle_rad - end_angle_rad) <= epsilon)
                    return;
                start_angle_rad = math::mod(start_angle_rad, two_pi);
                end_angle_rad = math::mod(end_angle_rad, two_pi);
                auto delta = end_angle_rad - start_angle_rad;
                if (!anti_clockwise) {
                    if (delta < zero) {
                        end_angle_rad += two_pi;
                    }
                } else {
                    if (delta > zero) {
                        end_angle_rad -= two_pi;
                    }
                }
                // after the moduli operation we might have done a%a which is equal to zero
                if (microgl::math::abs(delta) <= epsilon)
                    end_angle_rad = start_angle_rad + two_pi;

                // todo:: add minimal arc length fixer
                delta = (end_angle_rad - start_angle_rad) / number(divisions);

                auto radians = start_angle_rad;
                auto rotation_sin = math::sin(rotation);
                auto rotation_cos = math::cos(rotation);
                for (index ix = 0; ix <= divisions; ++ix) {
                    auto sine = math::sin(radians);
                    auto cosine = math::sin(radians + half_pi);
                    // currently , we assume pixel coords and radians have the same precision
                    // this probably is not a very good idea
                    auto x = cosine * radius_x;
                    auto y = sine * radius_y;
                    // now apply rotation
                    auto rotated_x= x*rotation_cos - y*rotation_sin;
                    auto rotated_y= x*rotation_sin + y*rotation_cos;
                    output.push_back({(center_x + rotated_x),
                                      (center_y + rotated_y)});
                    radians += delta;
                }
            }

        private:
        };


    }

}
