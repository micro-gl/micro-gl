#include <microgl/tesselation/ArcDivider.h>

namespace tessellation {

    int compute_sin_fixed(const int radians,
                          const precision precision) {
        auto precision_m = float(1u<<precision);
        float radians_f = float(radians) / precision_m;

        return int(std::sin(radians_f)*precision_m);
    }

    int compute_cos_fixed(const int radians,
                          const precision precision) {
        const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<precision));
        return compute_sin_fixed(radians + half_pi_fixed, precision);
    }

    template<typename T>
    void ArcDivider::swap(T &a, T &b) {
        T temp = a;
        a = b;
        b = temp;
    }

    ArcDivider::ArcDivider(bool DEBUG) {

    }

    void ArcDivider::compute(index radius,
                             int start_angle,
                             int end_angle,
                             const precision precision,
                             const index divisions,
                             bool clockwise,
                             array_container<vec2_32i> &output) {

        using very_long = long long;
        using uint = unsigned int;
        const auto two_pi_fixed = uint(TWO_PI * float(1u<<precision));
        const auto half_pi_fixed = uint(HALF_PI * float(1u<<precision));

        uint start_angle_positive = start_angle % two_pi_fixed;
        uint end_angle_positive = end_angle % two_pi_fixed;

        if(!clockwise)
            swap(start_angle_positive, end_angle_positive);

        uint delta = (end_angle_positive - start_angle_positive)/divisions;
        uint radians = uint(start_angle_positive);

        for (index ix = 0; ix < divisions; ++ix) {
            int sine = compute_sin_fixed(radians, precision);
            int cosine = compute_sin_fixed(radians+half_pi_fixed, precision);

            sine = (very_long(sine)*radius)>>precision;
            cosine = (very_long(cosine)*radius)>>precision;

            output.push_back({sine, cosine});

            radians += delta;
        }

    }

}