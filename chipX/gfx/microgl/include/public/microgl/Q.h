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

    inline
    integer convert_q_value_to_my_space(const_ref q) {
        return convert(q.value(), q.precision, this->precision);
    }

public:
    static const precision_t precision = P;
    integer _value = 0;

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
    Q(const_signed_ref q_val, precision_t q_precision) {
        this->_value = convert(q_val, q_precision, P);
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
    q_ref operator =(const float &float_value) {
        return (*this)=Q{float_value};
    }
    q_ref operator *=(const_ref q) {
        long long inter = ((long long)this->_value*q.value());
        this->_value = inter>>P;
        return *this;
    }
    q_ref operator /=(const_ref q) {
        this->_value = (_value<<P)/q.value();
        return *this;
    }
    q_ref operator +=(const_ref q) {
        this->_value+=convert_q_value_to_my_space(q);
        return *this;
    }
    q_ref operator -=(const_ref q) {
        this->_value -= convert_q_value_to_my_space(q);
        return *this;
    }

    // int assignments
    q_ref operator *=(const_signed_ref i) {
        this->_value*=i;
        return *this;
    }
    q_ref operator /=(const_signed_ref i) {
        this->_value/=i;
        return *this;
    }
    q_ref operator +=(const_signed_ref i) {
        this->_value+=(i<<P);
        return *this;
    }
    q_ref operator -=(const_signed_ref i) {
        this->_value -= (i<<P);
        return *this;
    }

    // intermediate Q
    Q operator +(const_ref q) const {
        Q temp{*this}; temp+=q;
        return temp;
    }
    Q operator *(const_ref q) const {
        Q temp{*this}; temp*=q;
        return temp;
    }
    Q operator /(const_ref q) const {
        Q temp{*this}; temp/=q;
        return temp;
    }
    Q operator -(const_ref q) const {
        Q temp{*this}; temp-=q;
        return temp;
    }

    // intermediate int
    Q operator +(const_signed_ref i) const {
        Q temp{this->_value + (i<<P), P};
        return temp;
    }
    Q operator *(const_signed_ref i) const {
        Q temp{this->_value*i, P};
        return temp;
    }
    Q operator /(const_signed_ref i) const {
        Q temp{this->_value/i, P};
        return temp;
    }
    Q operator -(const_signed_ref i) const {
        Q temp{this->_value - (i<<P), P};
        return temp;
    }

    // negate
    Q operator -() const {
        return Q{-this->value(), P};
    }

    // booleans
    bool operator <(const_ref q) const {
        return this->_value<q._value;
    }
    bool operator >(const_ref q) const {
        return this->_value>q._value;
    }
    bool operator <=(const_ref q) const {
        return this->_value<=q._value;
    }
    bool operator >=(const_ref q) const {
        return this->_value>=q._value;
    }
    bool operator ==(const_ref q) const {
        return this->_value==q._value;
    }
    bool operator !=(const_ref q) const {
        return this->_value!=q._value;
    }
    explicit operator bool() const {
        return this->_value!=0;
    }

    Q operator %(const_ref q) const {
        Q res;
        res._value = this->value()%q.value();
        return res;
    }

    // conversion operators
    explicit operator float() const {
        return toFloat();
    }
    explicit operator int() const {
        return toInt();
    }
    explicit operator long long() const {
        return toInt();
    }
    explicit operator unsigned char() const {
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
    // bypass all
    inline void updateValue(const_signed_ref val) {
        this->_value=val;
    }

};
