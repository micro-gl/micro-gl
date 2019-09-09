#pragma once

template <unsigned N>
class Q {
private:
    using index = unsigned int;
    using precision_t = unsigned char;
    using s32 = int;
    using const_ref = const Q &;
    using const_exact_ref = const Q<N> &;
    using const_signed_ref = const int &;
    using q_ref = Q &;
    // todo:: this can be removed if I only stick to N
    precision_t _precision = N;
    long _value = 0;
//    long _inter = 0;

    static inline
    int scale(long long from_value,
              precision_t from_precision,
              precision_t to_precision) {
        if(from_precision==to_precision)
            return from_value;
        else if(from_precision>to_precision)
            return (from_value)>>(from_precision - to_precision);
        else
            return (from_value)<<(to_precision - from_precision);
    }

    inline
    int scale_q_value_to_my_space(const_ref q) {
        return scale(q.value(), q.precision(), this->precision());
    }

public:
    Q() {
        this->_value = 0;
    }

    Q(const_ref q) {
        this->_value = scale(q.value(),
                             q.precision(),
                             _precision);
    }

    // this is Q<0>, so we promote it to Q<N>
    Q(const_signed_ref int_val) {
        this->_value = int_val<<this->precision();
    }

    Q(const float &float_val) {
        this->_value = int(float_val * float(1u << this->precision()));
    }

    // with assignments
    q_ref operator =(const_ref q) {
        this->_value = scale(q.value(),
                             q.precision(),
                             _precision);
        return *this;
    }
    // with assignments
    // this is Q<N>
    q_ref operator =(const_signed_ref signed_value) {
        this->_value = signed_value;//<<this->precision();
        return *this;
    }
    q_ref operator =(const float &float_value) {
        return (*this)=Q{float_value};
    }
    q_ref operator *=(const_ref q) {
        long long inter = ((long long)this->_value*q.value());
        this->_value = scale(inter,
                this->precision() + q.precision(),
                this->precision());
        return *this;
    }
    q_ref operator /=(const_ref q) {
        long long inter = ((long long)this->_value)<<q.precision();
        this->_value = inter/q.value();
        return *this;
    }
    q_ref operator +=(const_ref q) {
        this->_value += scale_q_value_to_my_space(q);
        return *this;
    }
    q_ref operator -=(const_ref q) {
        this->_value -= scale_q_value_to_my_space(q);
        return *this;
    }

    // unsigned assignments
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
    Q operator *(const_ref q) const {
        Q temp{*this};
        temp *= q;
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

    // intermediate unsigned
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
        return Q{int(-this->value())};
    }

    template <unsigned P>
    static Q<P> fromInteger(int val) {
        return Q<P>{val<<P};
    }
    // convert
    int toInt() const {
        return this->_value>>(this->_precision);
    }

    float toFloat() const {
        return float(this->_value)/float(1u<<this->_precision);
    }

    long fraction() const {
        return _value & ((1u<<_precision) - 1);
    }

    long integral() const {
        return _value & (((1u<<_precision) - 1)<<_precision);
    }

    inline precision_t precision() const {
        return _precision;
    }

    inline long value() const {
        return _value;
    }

};