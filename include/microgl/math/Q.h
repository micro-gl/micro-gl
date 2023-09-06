/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

namespace detail_Q {
    template<int T>
    struct identity { constexpr static int N = T; };
}

/**
 *
 * @tparam P
 * @tparam integer
 * @tparam inter_integer
 * @tparam multiplication_strategy -1 = infer automatically (will not use the unsafe mul),
 *                                  0 = (faster, accurate, big-inter)
 *                                  1 = (slower, accurate, small-inter)
 *                                  2 = (fastest, inaccurate for signed right shift, big-inter)
 */
template <unsigned P,
        typename integer=int,
        typename inter_integer=long long,
        char multiplication_strategy=-1>
class Q {
public:
    using precision_t = unsigned char;
    using const_ref = const Q &;
    using q_ref = Q &;
    static constexpr unsigned precision = P;
    static constexpr integer MASK_FRAC_BITS = (integer(1)<<P)-1;
    static constexpr integer MASK_INT_BITS = ~MASK_FRAC_BITS;

    static constexpr char intermid_size = sizeof(integer);
    static constexpr char integer_size = sizeof(inter_integer);
    static constexpr char recommended_mul_strategy =
            ((intermid_size>integer_size) || integer_size==8) ? 2 : 1;
    static constexpr char inferred_mul_strategy = multiplication_strategy==-1 ?
                                                  recommended_mul_strategy : multiplication_strategy;
    static constexpr detail_Q::identity<inferred_mul_strategy> inferred_token{};

private:
    integer _value;

    /**
     * this right shift handles negative/positives values, but especially for
     * negative values, due to right shifting negative values results in arithmetic
     * shift, i.e (-1)>>4 == -1, which should be 0.
     */
    template<typename integer_type>
    static inline integer_type shift_right_correctly_by(const integer_type & val,
                                                        const unsigned by_bits) {
        static constexpr integer_type size_bits = (sizeof(integer_type)<<3)-1;
        const integer_type sign_id = val>>size_bits;
        integer_type result = (val^sign_id) - sign_id; // abs value to avoid arithmetic shift
        result = result>>by_bits; // right shift
        result = (result^sign_id) - sign_id; // un-abs to restore sign
        return result;
    }

    inline void multiply(const integer val) {
        _multiply(val, inferred_token);
    }
    inline void _multiply(const integer val, detail_Q::identity<0>) {
        inter_integer inter = ((inter_integer)_value)*val;
        _value = shift_right_correctly_by<inter_integer>(inter, P);
    }
    inline void _multiply(const integer val, detail_Q::identity<1>) {
        using int_t = inter_integer;
        const int_t fpValue1 = _value;
        const int_t fpValue2 = val;
        // no need to do non-arithmetic shift because it is part
        // of a big picture
        const int_t intPart1 = fpValue1>>P, intPart2 = fpValue2>>P;
        const int_t fracPart1 = fpValue1 & int_t(MASK_FRAC_BITS);
        const int_t fracPart2 = fpValue2 & int_t(MASK_FRAC_BITS);
        _value = int_t(((intPart1 * intPart2)<<P) + (intPart1 * fracPart2) +
                       (fracPart1 * intPart2) + (((fracPart1 * fracPart2)>>P) & int_t(MASK_FRAC_BITS)));
    }
    inline void _multiply(const integer val, detail_Q::identity<2>)
    { _value = (((inter_integer)_value)*val)>>P; }

public:

    template<unsigned from_precision, unsigned to_precision>
    static inline integer convert_compile_time_variant(const integer from_value) {
        // the constexpr will eliminate the branching at compile time with simple compiler optimization
        constexpr auto delta = from_precision - to_precision;
        if(delta==0) return from_value;
        else if(delta>0) return shift_right_correctly_by<integer>(from_value, delta);
        else return (from_value<<(-delta));
    }

    static inline integer convert_runtime_variant(const integer from_value, precision_t from_precision,
                                                  precision_t to_precision) {
        const auto delta = from_precision-to_precision;
        if(delta==0) return from_value;
        else if(delta>0) return shift_right_correctly_by<integer>(from_value, delta);
        else return (from_value<<(-delta));
    }

    // constructors
    Q() : _value(0) {};
    Q(const_ref q) : _value{integer(q._value)} {}
    // conversion constructor, this reduces many other
    template <unsigned P_2, typename c1, typename c2, char s>
    Q(const Q<P_2, c1, c2, s> &q) : _value(convert_compile_time_variant<P_2, P>(q._value)) {}
    Q(const integer & q_val, precision_t q_precision) :
            _value(convert_runtime_variant(q_val, q_precision, P)) {}
    // this is Q<0>, so we promote it to Q<P>
    Q(const unsigned val) : _value(integer(val)<<P) {}
    Q(const signed val) : _value(val<<P) {}
    Q(const float val) : _value(integer(val * float(1u<<P))) {}
    Q(const double val) : _value(integer(val * double(1u<<P))) {}

    // with assignments operators
    q_ref operator =(const_ref q) { _value = q.value(); return *this; }
    q_ref operator *=(const_ref q) {
        multiply(q.value()); return *this;
//        multiply<inferred_mul_strategy>(q.value()); return *this;
    }
    q_ref operator *=(unsigned val) {_value *= val; return *this;}
    q_ref operator *=(signed val) {_value *= val; return *this;}
    q_ref operator /=(const_ref q) {
        const inter_integer inter=(inter_integer(_value)) << P;
        _value = ((inter_integer(_value)) << P)/(inter_integer)q.value();
        return *this;
    }
    q_ref operator /=(unsigned val) {_value /= val; return *this;}
    q_ref operator /=(signed val) {_value /= val; return *this;}
    q_ref operator +=(const_ref q) { _value+=q.value(); return *this; }
    q_ref operator -=(const_ref q) { _value-=q.value(); return *this; }

    // intermediate Q
    Q operator +(const_ref q) const { Q temp{*this}; temp+=q; return temp; }
    Q operator -(const_ref q) const { Q temp{*this}; temp-=q; return temp; }
    Q operator *(const_ref q) const { Q temp{*this}; temp*=q; return temp; }
    Q operator *(unsigned val) const { Q temp{*this}; temp._value*=val; return temp; }
    Q operator *(signed val) const { Q temp{*this}; temp._value*=val; return temp; }
    Q operator /(const_ref q) const { Q temp{*this}; temp/=q; return temp; }
    Q operator /(unsigned val) const { Q temp{*this}; temp._value/=val; return temp; }
    Q operator /(signed val) const { Q temp{*this}; temp._value/=val; return temp; }

    // negate
    Q operator -() const { return Q{0}.updateValue(-_value); }

    // booleans
    bool operator <(const_ref q) const { return _value<q._value; }
    bool operator >(const_ref q) const { return _value>q._value; }
    bool operator <=(const_ref q) const { return _value<=q._value; }
    bool operator >=(const_ref q) const { return _value>=q._value; }
    bool operator ==(const_ref q) const { return _value==q._value; }
    bool operator !=(const_ref q) const { return _value!=q._value; }

    // modolu
    Q operator %(const_ref q) const { return Q().updateValue(_value%q._value); }

    // conversion operators
    explicit operator bool() const { return _value!=0; }
    explicit operator float() const { return toFloat(); }
    explicit operator double() const { return toFloat(); }
    explicit operator short() const { return toInt(); }
    explicit operator unsigned short() const { return toInt(); }
    explicit operator int() const { return toInt(); }
    explicit operator unsigned int() const { return toInt(); }
    explicit operator long long() const { return toInt(); }
    explicit operator unsigned long long() const { return toInt(); }
    explicit operator char() const { return toInt(); }
    explicit operator unsigned char() const { return toInt(); }

    integer toInt() const { return shift_right_correctly_by<integer>(_value, P); }
    integer toFixed(const precision_t precision_value) const
    { return convert_runtime_variant(_value, P, precision_value); }
    template<unsigned precision_value>
    integer toFixedFaster() const { return convert_compile_time_variant<P, precision_value>(_value); }
    float toFloat() const { return float(_value)/float(1u<<P); }
    integer raw_integral() const { return (_value>>P); }
    integer raw_fraction() const { return _value & MASK_FRAC_BITS; }
    integer integral() const { return toInt(); }
    integer fraction() const { return (_value<0?-_value:_value)&MASK_FRAC_BITS; }
    inline integer value() const { return _value; }
    q_ref updateValue(const integer & val) { _value=val; return (*this); }
    Q abs() const { return _value<0? (*this):-(*this); }
    Q mod(const_ref val) const { return (*this)%val; }
    Q sqrt() const {
        Q val = *this, x = val, y = Q(1);
        const Q two(2), epsilon=Q(1)/Q(1u<<P);
        while ((x - y).abs() > epsilon)
        { x = (x + y) / two; y = val / x; }
        return x;
    }
};