#include <microgl/Q.h>
#include <microgl/math/std_q_math.h>
#include <iostream>

template<typename number_type>
number_type change_bit(const number_type & number,
                const number_type & x, unsigned char bit) {
    return (number & (~(number_type(1) << bit))) | (x << bit);
}

template<typename number_type>
number_type clear_bit(const number_type & number,
               unsigned char bit) {
    return (number & (~(number_type(1) << bit)));
}

template<typename number_type>
number_type get_bit(const number_type & number,
                    unsigned char bit) {
    return (number >> bit) & (number_type(1));
}

template<typename number_type>
number_type toggle_bit(const number_type & number,
                       unsigned char bit) {
    return number ^ (number_type(1) << bit);
}

template<typename number_type>
int abs_2(const number_type & x) {
    constexpr auto bit = (sizeof(number_type)<<3)-1;
    number_type y = (x >> bit);
    return (x ^ y) - y;
}

int main() {
    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;

    using q = Q<16, int32_t>;

    q a{-4}, b{4};
    q c = a*b;
    a.updateValue(-1);
    int d = int(a);
    int d2 = a.fraction();
    int val= -4;
    using integer = decltype(val);
    unsigned char bit = (sizeof(integer)<<3)-1;
    integer sign_bit_val = get_bit<integer>(val, bit);
    integer abs = clear_bit<integer>(val, bit);

    int abs3 = abs_2(val);
    int abs4 = ~abs - (1<<bit);
    auto tt = int(-1)>>16;
    integer sign_bit_val2 = get_bit<integer>(tt, 0);
    integer sign_bit_val33 = get_bit<integer>(tt, 31);

    // test
    integer train = val>>31;
    int abs_1 = (val^train) - train;
    int un_abs_1 = (abs_1^train) - train;



}

