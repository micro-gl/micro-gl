#pragma once

namespace microgl {
    namespace color {

        template <typename number=float>
        struct intensity {
            intensity(const number &r=number(0),
                      const number &g=number(0),
                      const number &b=number(0),
                      const number &a=number(1)) :
                            r{r}, g{g}, b{b}, a{a} {}
            number r=0, g=0, b=0, a=number(1);
        };
    }
}
