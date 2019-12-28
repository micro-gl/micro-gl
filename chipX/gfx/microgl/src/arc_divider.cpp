//#include <microgl/tesselation/arc_divider.h>

namespace tessellation {

    template <typename number>
    void arc_divider<number>::compute(dynamic_array<vertex> &output,
                                      number radius,
                                      number center_x,
                                      number center_y,
                                      number start_angle_rad,
                                      number end_angle_rad,
                                      const int divisions,
                                      bool anti_clockwise) {

        const auto two_pi = math::pi<number>()*number(2);
        const auto half_pi = math::pi<number>()/number(2);
        const auto zero = number(0);
        number epsilon = number(1)/number(100);
        if(microgl::math::abs_(start_angle_rad - end_angle_rad) <= epsilon)
            return;

        start_angle_rad = math::mod(start_angle_rad, two_pi);
        end_angle_rad = math::mod(end_angle_rad, two_pi);
        auto delta = end_angle_rad - start_angle_rad;

        if(!anti_clockwise) {
            if(delta < zero) {
                end_angle_rad += two_pi;
            }
        } else {
            if(delta > zero) {
                end_angle_rad -= two_pi;
            }
        }
        // after the moduli operation we might have done a%a which is equal to zero
        if(microgl::math::abs_(delta) <= epsilon)
            end_angle_rad = start_angle_rad + two_pi;

        delta = (end_angle_rad - start_angle_rad) / number(divisions);

        auto radians = start_angle_rad;

        for (index ix = 0; ix <= divisions; ++ix) {

            auto sine = math::sin(radians);
            auto cosine = math::sin(radians + half_pi);

            // currently , we assume pixel coords and radians have the same precision
            // this probably is not a very good idea
            sine = sine*radius;
            cosine = cosine*radius;

            output.push_back({(center_x + cosine),
                              (center_y + sine)});

            radians += delta;
        }

    }

}