#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/array_container.h>
#include <microgl/trigo_functions.h>

namespace tessellation {
#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268
#define TWO_PI    6.28318530718
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))
#define abs_(a) ((a)<0 ? -(a) : (a))
#define clamp_(v, e0, e1) (min_(max_(v,e0),e1))

    using index = unsigned int;
    using precision_t = unsigned char;
    using namespace microgl;

    class ArcDivider2 {
    public:

        explicit ArcDivider2(bool DEBUG = false){};;

        template <typename number>
        static void compute(
                     array_container<vec2<number>> &output,
                     number radius,
                     number center_x,
                     number center_y,
                     number start_angle_rad,
                     number end_angle_rad,
                     precision_t precision_angles,
                     index divisions=32,
                     bool anti_clockwise=false
                     );

    private:

        bool _DEBUG = false;
    };

    template<typename number>
    void ArcDivider2::compute(array_container<vec2<number>> &output,
                              number radius,
                              number center_x,
                              number center_y,
                              number start_angle_rad,
                              number end_angle_rad,
                              precision_t precision_angles,
                              index divisions,
                              bool anti_clockwise) {


        using very_long = long long;
        using uint = unsigned int;
        const precision_t sine_precision = 15;
        const precision_t angle_to_sine_precision = sine_precision - precision_angles;
        const auto two_pi_fixed = uint(TWO_PI * float(1u<<sine_precision));
        const auto half_pi_fixed = uint(HALF_PI * float(1u<<sine_precision));

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
        }

        delta = (int(end_angle_rad) - int(start_angle_rad)) / int(divisions);

        int radians = (start_angle_rad);

        for (index ix = 0; ix <= divisions; ++ix) {
            int sine = microgl::functions::sin(radians, sine_precision, sine_precision);
            int cosine = microgl::functions::sin(radians+half_pi_fixed, sine_precision, sine_precision);

            sine = (very_long(sine)*radius)>>(sine_precision);
            cosine = (very_long(cosine)*radius)>>(sine_precision);

            output.push_back({(center_x + cosine)>>0,
                              (center_y + sine)>>0});

            radians += delta;
        }

    }


}

#pragma clang diagnostic pop