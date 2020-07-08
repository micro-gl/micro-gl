#pragma once

#include <microgl/micro_gl_traits.h>

template <typename type, unsigned size, typename function>
struct constexpr_lut {
private:
    struct Table {
        type data[size];
    };

    template<int... Is> struct seq{};

    template<int N, int... Is>
    struct gen_seq : gen_seq<(N)-(1), (N)-(1), (Is)...>{};

    template<int... Is>
    struct gen_seq<(0), Is...> : seq<(Is)...>{};

    template<>
    struct gen_seq<(0)> : seq<0>{};

    template<int... Is>
    static constexpr Table generate(seq<Is...>){
        return {{ function::apply(Is)... }};
    }

    constexpr static Table tab=generate(gen_seq<size>{});

public:
    static type get(const int & n) {
        return tab.data[n];
    }

};

// definition
template <typename type, unsigned size, typename function>
constexpr typename constexpr_lut<type, size, function>::Table constexpr_lut<type, size, function>::tab;

template <uint8_t bits1, uint8_t bits2>
struct lut_bits {
    // let's fail if more than 8 bits
    typename microgl::traits::enable_if<bits1<=8 && bits2<=8, bool>::type fail_if_more_than_8_bits;
    constexpr static unsigned size=1u<<bits1;
    lut_bits()= delete;

    struct func {
        // this is the function to generate the LUT elements
        static constexpr uint8_t apply(int n) {
            const int max_val_1 = (int(1)<<bits1)-1;
            const int max_val_2 = (int(1)<<bits2)-1;
            int inter= bits1==0 ? 0 : (n*max_val_2)/max_val_1;
            return (inter);
        }
    };

    static uint8_t get(const int & n) {
        return lut.get(n);
    }
private:
    const static constexpr_lut<uint8_t, size, func> lut;
};

// definition
template <uint8_t bits1, uint8_t bits2>
const constexpr_lut<uint8_t, lut_bits<bits1, bits2>::size, typename lut_bits<bits1, bits2>::func> lut_bits<bits1, bits2>::lut;
