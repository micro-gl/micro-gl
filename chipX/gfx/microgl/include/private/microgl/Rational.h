#pragma once
#include <stdexcept>

struct Rational {

    mutable long numerator = 0, denominator=1;

    Rational();
    Rational(long n);
    Rational(long n , long d);
    Rational(const Rational & val);

    Rational & operator=(const Rational & val);
    Rational & operator=(const long & val);

    float toFloat() const;;
    long toFixed(unsigned char precision = 0) const;;

    bool isRegular() const;
    bool isIrregular() const;
    bool isUndefined() const;
    bool isDefined() const;
    bool isPositiveInfinity() const;
    bool isNegativeInfinity() const;
    bool isPositive() const;
    bool isNegative() const;


    static const Rational maximum(const Rational &lhs,
                                  const Rational &rhs);
    static const Rational minimum(const Rational &lhs,
                                  const Rational &rhs);
    static const Rational clamp(const Rational &a,
                                const Rational &left,
                                const Rational &right);
    Rational absolute() const;
    void makeDenominatorPositive() const;
    void throwIfDenomIsZero() const;

    Rational operator*(const long & val) const;
    Rational operator*(const Rational & val) const;
    Rational operator/(const long & val) const;
    Rational operator/(const Rational & val) const;
    Rational operator+(const long & val) const;
    Rational operator+(const Rational & val) const;
    Rational operator-(const long & val) const;
    Rational operator-() const;
    Rational operator-(const Rational & val) const;

    // boolean operations here

    bool operator!=(const long & rhs) const;
    bool operator==(const long & rhs) const;
    bool operator==(const Rational & rhs) const;
    bool operator!=(const Rational & val) const;
    bool operator <= (const Rational & rhs) const;
    bool operator >= (const Rational & rhs);
    bool operator < (const Rational & rhs) const;
    bool operator > (const Rational & rhs) const;
};
