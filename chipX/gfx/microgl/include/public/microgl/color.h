#pragma once

namespace microgl {
    namespace color {
        using channel = unsigned char;
        using bits = unsigned char;

        typedef struct {
            channel r=0, g=0, b=0, a=0;
            bits r_bits=8, g_bits=8, b_bits=8, a_bits=8;
        } color_t;

        struct color_f_t {
            float r, g, b, a;
        };

        class colors {
        public:
            static color_f_t RED;
            static color_f_t YELLOW;
            static color_f_t WHITE;
            static color_f_t GREEN;
            static color_f_t BLUE;
            static color_f_t BLACK;

        };

    }

}
