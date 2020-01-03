#pragma once

template <unsigned P>
class Q {
private:
    using index = unsigned int;
    using integer = long long;
    using precision_t = unsigned char;
    using const_ref = const Q &;
    using const_exact_ref = const Q<P> &;
    using const_signed_ref = const integer &;
    using q_ref = Q &;
    integer _value = 0;
//    int _value = 0;

    inline
    integer convert_q_value_to_my_space(const_ref q) {
        return convert(q.value(), q.precision, this->precision);
    }

public:
    static const precision_t precision = P;

    static inline
    integer convert(long long from_value,
                precision_t from_precision,
                precision_t to_precision) {
        if(from_precision==to_precision)
            return from_value;
        else if(from_precision>to_precision)
            return (from_value)>>(from_precision - to_precision);
        else
            return (from_value)<<(to_precision - from_precision);
    }

    Q() {
        this->_value = 0;
    }

    Q(const_ref q) {
        this->_value = q.value();
    }

    // conversion constructor, this reduces many other
    // operators.
    template <unsigned P_2>
    Q(const Q<P_2> &q) {
        this->_value = convert(q.value(),
                             P_2,
                             P);
    }

    // this is Q<0>, so we promote it to Q<P>
    Q(const_signed_ref int_val) {
        this->_value = int_val<<P;
    }

    Q(const signed& int_val) {
        this->_value = int_val<<P;
    }

    Q(const unsigned & int_val) {
        this->_value = int_val<<P;
    }

    Q(const_signed_ref q_val, precision_t q_precision) {
        this->_value = convert(q_val,
                             q_precision,
                             P);
    }

    Q(const float &val) {
        this->_value = integer(val * float(1u<<P));
    }

    Q(const double &val) {
        this->_value = integer(val * float(1u<<P));
    }

    // with assignments operators
    q_ref operator =(const_ref q) {
        this->_value = q.value();
        return *this;
    }
    // with assignments
    // this is Q<P>
    // note:: for assignment with integer we assume, that it is already in P bits space
    // todo:: see if I am actually using it. this might have unwanted side effects
//    q_ref operator =(const_signed_ref signed_value) {
//        this->_value = signed_value;
//        return *this;
//    }
    q_ref operator =(const float &float_value) {
        return (*this)=Q{float_value};
    }
    template <unsigned P_2>
    q_ref operator *=(const Q<P_2> &q) {
        long long inter = ((long long)this->_value*q.value());
        this->_value = inter>>P_2;
        return *this;
    }
    q_ref operator *=(const_ref q) {
        long long inter = ((long long)this->_value*q.value());
        this->_value = inter>>P;
        return *this;
    }
    template <unsigned P_2>
    q_ref operator /=(const Q<P_2> &q) {
        long long inter = ((long long)this->_value)<<P_2;
        this->_value = inter/q.value();
        return *this;
    }
    q_ref operator /=(const_ref q) {
        long long inter = ((long long)this->_value)<<P;
        this->_value = inter/q.value();
        return *this;
    }
    q_ref operator +=(const_ref q) {
        this->_value += convert_q_value_to_my_space(q);
        return *this;
    }
    q_ref operator -=(const_ref q) {
        this->_value -= convert_q_value_to_my_space(q);
        return *this;
    }

    // int assignments
    q_ref operator *=(const_signed_ref i) {
        this->_value *= i;
        return *this;
    }
    q_ref operator /=(const_signed_ref i) {
        this->_value /= i;
        return *this;
    }
    q_ref operator +=(const_signed_ref i) {
        this->_value += i;
        return *this;
    }
    q_ref operator -=(const_signed_ref i) {
        this->_value -= i;
        return *this;
    }

    // intermediate Q
    Q operator +(const_ref q) const {
        Q temp{*this};
        temp += q;
        return temp;
    }
    template <unsigned P_2>
    Q operator *(const Q<P_2> &q) {
        Q temp{*this};
        temp *= q;
        return temp;
    }
    Q operator *(const_ref q) const {
        Q temp{*this};
        temp *= q;
        return temp;
    }
    template <unsigned P_2>
    Q operator /(const Q<P_2> &q) {
        Q temp{*this};
        temp /= q;
        return temp;
    }
    Q operator /(const_ref q) const {
        Q temp{*this};
        temp /= q;
        return temp;
    }
    Q operator -(const_ref q) const {
        Q temp{*this};
        temp -= q;
        return temp;
    }

    // intermediate int
    Q operator +(const_signed_ref i) const {
        Q temp{*this};
        temp += i;
        return temp;
    }
    Q operator *(const_signed_ref i) const {
        Q temp{*this};
        temp *= i;
        return temp;
    }
    Q operator /(const_signed_ref i) const {
        Q temp{*this};
        temp /= i;
        return temp;
    }
    Q operator -(const_signed_ref i) const {
        Q temp{*this};
        temp -= i;
        return temp;
    }

    // negate
    Q operator -() const {
        return Q{-this->value(), P};
    }

    // booleans
//    template <unsigned P_2>
    bool operator <(const_ref q) const {
        return this->_value<q._value;
    }
//    template <unsigned P_2>
    bool operator >(const_ref q) const {
        return this->_value>q._value;
    }
//    template <unsigned P_2>
    bool operator <=(const_ref q) const {
        return this->_value<=q._value;
    }
//    template <unsigned P_2>
    bool operator >=(const_ref q) const {
        return this->_value>=q._value;
    }
//    template <unsigned P_2>
    bool operator ==(const_ref q) const {
        return this->_value==q._value;
    }
    bool operator !=(const_ref q) const {
        return this->_value!=q._value;
    }
    explicit operator bool() const {
        return this->_value!=0;
    }

    Q operator % (const_ref q) const {
        Q res;
        res._value = this->value()%q.value();
        return res;
    }

    // conversion operators
    explicit operator float() {
        return toFloat();
    }
    explicit operator int() {
        return toInt();
    }

    integer toInt() const {
        return this->_value>>P;
    }

    integer toFixed(precision_t precision_value) const {
        return convert(this->_value, P, precision_value);
    }

    float toFloat() const {
        return float(this->_value)/float(1u<<P);
    }

    integer fraction() const {
        return _value & ((1u<<P) - 1);
    }

    integer integral() const {
        return _value & (((1u<<P) - 1)<<P);
    }

    inline integer value() const {
        return _value;
    }

};

//namespace microgl {
//    namespace math {
////        template <>
////        int to_fixed(const Q<8> & val, unsigned char precision) {
////            return int(val.toFixed(precision));
////        }
//        template<unsigned N>
//        int ato_fixed(const Q<N> & val, unsigned char precision) {
//            return int(val.toFixed(precision));
//        }
//    }
//}