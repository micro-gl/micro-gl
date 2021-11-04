#pragma once

// todo: use in-place overflow-less method
template <unsigned P, typename container_integer=long long,
        typename intermediate_container_integer=container_integer>
class Q {
public:
    using integer = container_integer;
    using precision_t = unsigned char;
    using const_ref = const Q &;
    using const_signed_ref = const integer &;
    using q_ref = Q &;
    static constexpr unsigned precision = P;
    static constexpr integer MASK_FRAC_BITS = (integer(1)<<P)-1;
    static constexpr integer MASK_INT_BITS = ~MASK_FRAC_BITS;

private:

    /**
     * this right shift handles negative/positives values, but especially for
     * negative values, due to right shifting negative values results in arithmetic
     * shift, i.e (-1)>>4 == -1, which should be 0.
     */
    template<typename integer_type>
    static inline integer_type shift_right_correctly_by_P(const integer_type & val) {
        static constexpr integer_type bits = (sizeof(integer_type)<<3)-1;
        const integer_type sign_id = val>>bits;
        integer_type result = (val^sign_id) - sign_id; // abs value to avoid arithmetic shift
        result = result>>P; // right shift
        result = (result^sign_id) - sign_id; // un-abs to restore sign
        return result;
    }

    inline integer convert_q_value_to_my_space(const_ref q) {
        return convert(q.value(), q.precision, this->precision);
    }
    template<class number> static inline number abs(const number & val) {
        return val<0?-val:val;
    }

public:
    integer _value;

    static inline
    integer convert(integer from_value,
                precision_t from_precision,
                precision_t to_precision) {
        if(from_precision==to_precision) return from_value;
        else {
            const bool isNegative=from_value<0;
            integer from_value_abs=abs(from_value);
            if(from_precision>to_precision) {
                integer inter=from_value_abs>>(from_precision - to_precision);
                return isNegative ? -inter : inter;
            } else {
                integer inter=from_value_abs<<(to_precision - from_precision);
                return isNegative ? -inter : inter;
            }
        }
    }

    // constructors
    Q() : _value(0) {};
    Q(const_ref q) : _value{integer(q.value())} {}
    // conversion constructor, this reduces many other
    template <unsigned P_2>
    Q(const Q<P_2> &q) : _value(convert(integer(q.value()), P_2, P)) {}
    Q(const_signed_ref q_val, precision_t q_precision) : _value(convert(q_val, q_precision, P)) {}
    // this is Q<0>, so we promote it to Q<P>
    Q(const unsigned int_val) : _value(integer(int_val)<<P) {}
    Q(const signed int_val) : _value(0) {
        const bool isNegative=int_val<0;
        this->_value = abs(integer(int_val))<<P;
        if(isNegative) this->_value = -this->_value;
    }
    Q(const float val) : _value(integer(val * float(1u<<P))) {}
    Q(const double val) : _value(integer(val * double(1u<<P))) {}

    // with assignments operators
    q_ref operator =(const_ref q) { this->_value = q.value(); return *this; }

//    q_ref operator *=(const_ref q) {
//        intermediate_container_integer inter = ((intermediate_container_integer)this->_value)*q.value();
//        this->_value = shift_right_correctly_by_P<intermediate_container_integer>(inter);
//        return *this;
//    }
    q_ref operator *=(const_ref q) {
        using int_t = integer;
        const int_t fpValue1 = _value;
        const int_t fpValue2 = q._value;
        // no need to do non-arithmatic shift because it is part
        // of a big picture
        const int_t intPart1 = fpValue1>>P, intPart2 = fpValue2>>P;
        const int_t fracPart1 = fpValue1 & MASK_FRAC_BITS;
        const int_t fracPart2 = fpValue2 & MASK_FRAC_BITS;

        _value = 0;
        _value += ((intPart1 * intPart2) << P);
        _value += (intPart1 * fracPart2);
        _value += (fracPart1 * intPart2);
        _value += (((fracPart1 * fracPart2)>>P) & MASK_FRAC_BITS);
        return *this;
    }
    q_ref operator /=(const_ref q) {
        const intermediate_container_integer inter=(intermediate_container_integer(_value))<<P;
        this->_value = inter/(intermediate_container_integer)q.value();
        return *this;
    }

//    q_ref operator *=(const_ref q) {
//        intermediate_container_integer inter = ((intermediate_container_integer)this->_value)*q.value();
//        this->_value = shift_right_correctly_by_P<intermediate_container_integer>(inter);
//        return *this;
//    }
//    q_ref operator *=(const_ref q) {
//        intermediate_container_integer inter = ((intermediate_container_integer)this->_value*q.value());
//        const bool isNegative=inter<0;
//        this->_value = abs<intermediate_container_integer>(inter)>>P;
//        if(isNegative) this->_value = -this->_value;
//        return *this;
//    }
    q_ref operator +=(const_ref q) { this->_value+=q.value(); return *this; }
    q_ref operator -=(const_ref q) { this->_value-=q.value(); return *this; }

    // intermediate Q
    Q operator +(const_ref q) const { Q temp{*this}; temp+=q; return temp; }
    Q operator -(const_ref q) const { Q temp{*this}; temp-=q; return temp; }
    Q operator *(const_ref q) const { Q temp{*this}; temp*=q; return temp; }
    Q operator /(const_ref q) const { Q temp{*this}; temp/=q; return temp; }

    // negate
    Q operator -() const { return Q{-this->value(), P}; }

    // booleans
    bool operator <(const_ref q) const { return this->_value<q._value; }
    bool operator >(const_ref q) const { return this->_value>q._value; }
    bool operator <=(const_ref q) const { return this->_value<=q._value; }
    bool operator >=(const_ref q) const { return this->_value>=q._value; }
    bool operator ==(const_ref q) const { return this->_value==q._value; }
    bool operator !=(const_ref q) const { return this->_value!=q._value; }
    Q operator %(const_ref q) const {
        Q res;
        res._value = this->value()%q.value();
        return res;
    }

    // conversion operators
    explicit operator bool() const { return this->_value!=0; }
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

    integer toInt() const { return shift_right_correctly_by_P(this->_value); }
    integer toFixed(precision_t precision_value) const { return convert(this->_value, P, precision_value); }
    float toFloat() const { return float(this->_value)/float(1u<<P); }
    integer raw_integral() const { return (this->_value>>P); }
    integer raw_fraction() const { return _value & MASK_FRAC_BITS; }
    integer integral() const { return toInt(); }
    integer fraction() const { return (_value<0?-_value:_value)&MASK_FRAC_BITS; }
    inline integer value() const { return _value; }
    inline void updateValue(const_signed_ref val) { this->_value=val; }

    Q<P> sqrt() const { return Q<P>((sqrt_<unsigned>(unsigned(_value))), P/2); }
    Q<P> abs() const { return _value<0? Q{-_value}:Q{_value}; }
    Q<P> mod(const_ref val) const { return (*this)%val; }

private:
    template<typename integer_type>
    static
    integer_type sqrt_(integer_type val) {
        constexpr unsigned char bits= sizeof (integer_type) * 8;
        integer_type op  = val;
        integer_type res = 0;
        // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type
        integer_type one = integer_type(1u) << (bits - 2);

        // "one" starts at the highest power of four <= than the argument.
        while (one > op) one >>= 2;
        while (one != 0) {
            if (op >= res + one) {
                op = op - (res + one);
                res = res + ( one<<1);
            }
            res >>= 1;
            one >>= 2;
        }
        return res;
    }
};

/*
 *
#pragma once

// todo: use in-place overflow-less method
template <unsigned P, typename container_integer=long long,
        typename intermediate_container_integer=long long>
class Q {
public:
    using integer = container_integer;
    using precision_t = unsigned char;
    using const_ref = const Q &;
    using const_signed_ref = const integer &;
    using q_ref = Q &;
    static constexpr unsigned precision = P;

private:
    inline
    integer convert_q_value_to_my_space(const_ref q) {
        return convert(q.value(), q.precision, this->precision);
    }

    template<class number> static inline number abs(const number & val) {
        return val<0?-val:val;
    }
    static inline integer abs2(const integer & val) { return val<0?-val:val; }
    static inline integer sign(const integer & val) { return val<0?-1:1; }

public:
    integer _value;

    static inline
    integer convert(integer from_value,
                precision_t from_precision,
                precision_t to_precision) {
        if(from_precision==to_precision)
            return from_value;
        else {
            const bool isNegative=from_value<0;
            integer from_value_abs=abs(from_value);
            if(from_precision>to_precision) {
                integer inter=from_value_abs>>(from_precision - to_precision);
                return isNegative ? -inter : inter;
            } else {
                integer inter=from_value_abs<<(to_precision - from_precision);
                return isNegative ? -inter : inter;
            }
        }
    }

    // constructors
    Q() : _value(0) {};
    Q(const_ref q) : _value{integer(q.value())} {}
    // conversion constructor, this reduces many other
    template <unsigned P_2>
    Q(const Q<P_2> &q) : _value(convert(integer(q.value()), P_2, P)) {}
    Q(const_signed_ref q_val, precision_t q_precision) : _value(convert(q_val, q_precision, P)) {}
    // this is Q<0>, so we promote it to Q<P>
    Q(const unsigned int_val) : _value(integer(int_val)<<P) {}
    Q(const signed int_val) : _value(0) {
        const bool isNegative=int_val<0;
        this->_value = abs(integer(int_val))<<P;
        if(isNegative) this->_value = -this->_value;
    }
    Q(const float val) : _value(integer(val * float(1u<<P))) {}
    Q(const double val) : _value(integer(val * double(1u<<P))) {}

    // with assignments operators
    q_ref operator =(const_ref q) { this->_value = q.value(); return *this; }
    q_ref operator *=(const_ref q) {
        intermediate_container_integer inter = ((intermediate_container_integer)this->_value*q.value());
        constexpr unsigned char bits = (sizeof(intermediate_container_integer)<<3)-1;
        const intermediate_container_integer sign_id = inter>>bits;
        inter = (inter^sign_id) - sign_id; // abs value to avoid arithmetic shift
        inter = inter>>P; // right shift
        inter = (inter^sign_id) - sign_id; // un-abs to restore sign
        this->_value = inter;
        return *this;
    }
    q_ref operator /=(const_ref q) {
        const bool isNegative=_value<0;
        intermediate_container_integer inter=(intermediate_container_integer(_value))<<P;
        this->_value = inter/(intermediate_container_integer)q.value();
        return *this;
    }

    //    q_ref operator *=(const_ref q) {
//        intermediate_container_integer inter = ((intermediate_container_integer)this->_value*q.value());
//        const bool isNegative=inter<0;
//        this->_value = abs<intermediate_container_integer>(inter)>>P;
//        if(isNegative) this->_value = -this->_value;
//        return *this;
//    }
//    q_ref operator /=(const_ref q) {
//        const bool isNegative=_value<0;
//        intermediate_container_integer value_abs=abs<intermediate_container_integer>(_value)<<P;
//        this->_value = value_abs/(intermediate_container_integer)q.value();
//        if(isNegative) this->_value = -this->_value;
//        return *this;
//    }
    q_ref operator +=(const_ref q) { this->_value+=q.value(); return *this; }
    q_ref operator -=(const_ref q) { this->_value-=q.value(); return *this; }

    // intermediate Q
    Q operator +(const_ref q) const { Q temp{*this}; temp+=q; return temp; }
    Q operator -(const_ref q) const { Q temp{*this}; temp-=q; return temp; }
    Q operator *(const_ref q) const { Q temp{*this}; temp*=q; return temp; }
    Q operator /(const_ref q) const { Q temp{*this}; temp/=q; return temp; }

    // negate
    Q operator -() const { return Q{-this->value(), P}; }

    // booleans
    bool operator <(const_ref q) const { return this->_value<q._value; }
    bool operator >(const_ref q) const { return this->_value>q._value; }
    bool operator <=(const_ref q) const { return this->_value<=q._value; }
    bool operator >=(const_ref q) const { return this->_value>=q._value; }
    bool operator ==(const_ref q) const { return this->_value==q._value; }
    bool operator !=(const_ref q) const { return this->_value!=q._value; }
    Q operator %(const_ref q) const {
        Q res;
        res._value = this->value()%q.value();
        return res;
    }

    // conversion operators
    explicit operator bool() const { return this->_value!=0; }
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

    integer toInt() const { return this->_value>>P; }
    integer toFixed(precision_t precision_value) const { return convert(this->_value, P, precision_value); }
    float toFloat() const { return float(this->_value)/float(1u<<P); }
    integer fraction() const { return _value & ((1u<<P) - 1); }
    integer integral() const { return _value & (~((1u<<P) - 1)); } //this is wrong ?}
    inline integer value() const { return _value; }
    inline void updateValue(const_signed_ref val) { this->_value=val; }

    Q<P> sqrt() const { return Q<P>((sqrt_<unsigned>(unsigned(_value))), P/2); }
    Q<P> abs() const { return _value<0? Q{-_value}:Q{_value}; }
    Q<P> mod(const_ref val) const { return (*this)%val; }

private:
    template<typename integer_type>
    static
    integer_type sqrt_(integer_type val) {
        constexpr unsigned char bits= sizeof (integer_type) * 8;
        integer_type op  = val;
        integer_type res = 0;
        // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type
        integer_type one = integer_type(1u) << (bits - 2);

        // "one" starts at the highest power of four <= than the argument.
        while (one > op) one >>= 2;
        while (one != 0) {
            if (op >= res + one) {
                op = op - (res + one);
                res = res + ( one<<1);
            }
            res >>= 1;
            one >>= 2;
        }
        return res;
    }
};

 */