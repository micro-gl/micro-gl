#include <microgl/tesselation/ArcDivider.h>

namespace tessellation {

    int compute_sin_fixed(const int radians,
                          precision_t input_precision,
                          precision_t output_precision) {
        auto precision_m = float(1u<<input_precision);
        auto precision_out = float(1u<<output_precision);
        float radians_f = float(radians) / precision_m;

        return int(std::sin(radians_f)*precision_out);
    }

    int compute_cos_fixed(const int radians,
                          const precision_t input_precision,
                          const precision_t output_precision) {
        const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<input_precision));
        return compute_sin_fixed(radians + half_pi_fixed, input_precision, output_precision);
    }

    template<typename T>
    void ArcDivider::swap(T &a, T &b) {
        T temp = a;
        a = b;
        b = temp;
    }

    ArcDivider::ArcDivider(bool DEBUG) {

    }

    void ArcDivider::compute(array_container<vec2_32i> &output,
                             index radius,
                             int center_x,
                             int center_y,
                             int start_angle_rad,
                             int end_angle_rad,
                             const precision_t precision_angles,
                             const index divisions,
                             bool anti_clockwise) {

        using very_long = long long;
        using uint = unsigned int;
        const precision_t sine_precision = 15;
        const precision_t angle_to_sine_precision = sine_precision - precision_angles;
        const auto two_pi_fixed = uint(TWO_PI * float(1u<<sine_precision));
        const auto half_pi_fixed = uint(HALF_PI * float(1u<<sine_precision));
        bool full_circle = false;
        if(start_angle_rad == end_angle_rad)
            return;

        start_angle_rad <<= angle_to_sine_precision;
        end_angle_rad <<= angle_to_sine_precision;

        start_angle_rad = (start_angle_rad) % int(two_pi_fixed);
        end_angle_rad = (end_angle_rad) % int(two_pi_fixed);

        int delta = int(end_angle_rad) - int(start_angle_rad);

        if(!anti_clockwise) {
            if(delta < 0) {
                end_angle_rad += two_pi_fixed;
            }
        } else {
            if(delta > 0) {
                end_angle_rad -= two_pi_fixed;
            }
        }

        if(delta==0) {
            end_angle_rad = start_angle_rad + two_pi_fixed;
            full_circle = true;
        }

        delta = (int(end_angle_rad) - int(start_angle_rad)) / int(divisions);

        int radians = (start_angle_rad);

        for (index ix = 0; ix <= divisions; ++ix) {
            int sine = compute_sin_fixed(radians, sine_precision, sine_precision);
            int cosine = compute_sin_fixed(radians+half_pi_fixed, sine_precision, sine_precision);

            sine = (very_long(sine)*radius)>>(sine_precision);
            cosine = (very_long(cosine)*radius)>>(sine_precision);

            output.push_back({(center_x + cosine)>>0,
                              (center_y + sine)>>0});

            radians += delta;
        }

    }

}