//
// Created by Tomer Shalev on 2019-08-14.
//
#include <Rational.h>

Rational::Rational() {}

Rational::Rational(long n) : Rational(n, 1) {
}

Rational::Rational(long n, long d) : numerator{n}, denominator{d} {
}

Rational::Rational(const Rational &val) {
    numerator = val.numerator;
    denominator = val.denominator;
}

Rational &Rational::operator=(const Rational &val) {
    if(this==&val)
        return *this;
    this->numerator = val.numerator;
    this->denominator = val.denominator;
    return *this;
}

Rational &Rational::operator=(const long &val) {
    numerator = val;
    denominator = 1;
    return *this;
}

float Rational::toFloat() const {
    return float(numerator)/float(denominator);
}

long Rational::toFixed(unsigned char precision) const {
    return (numerator<<precision)/(denominator);
}

bool Rational::isRegular() const {
    return denominator!=0;
}

bool Rational::isIrregular() const {
    return denominator!=0;
}

bool Rational::isUndefined() const {
    return numerator==0 && denominator==0;
}

bool Rational::isDefined() const {
    return !isUndefined();
}

bool Rational::isPositiveInfinity() const {
    // todo: might be wrong
    return numerator>=0 && denominator==0;
}

bool Rational::isNegativeInfinity() const {
    return numerator<0 && denominator==0;
}

const Rational Rational::maximum(const Rational &lhs, const Rational &rhs) {
    return (lhs<=rhs) ? rhs : lhs;
}

const Rational Rational::minimum(const Rational &lhs, const Rational &rhs) {
    return (lhs<=rhs) ? lhs : rhs;
}

const Rational Rational::clamp(const Rational &a, const Rational &left, const Rational &right) {
    Rational c = minimum(maximum(a, left), right);
    return c;
}

Rational Rational::absolute() const {
    long n = numerator;
    long d = denominator;

    if(isNegative()) {
        n = numerator<0 ? -numerator : numerator;
        d = denominator<0 ? -denominator : denominator;
    } else {

    }

    return Rational{n, d};
}

void Rational::makeDenominatorPositive() const {
    if(denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }

}

Rational Rational::operator*(const long &val) const {
    long n = numerator*val;
    return Rational{n, denominator};
}

Rational Rational::operator*(const Rational &val) const {
    throwIfDenomIsZero();

    long n = numerator*val.numerator;
    long d = denominator*val.denominator;
    return Rational{n, d};
}

Rational Rational::operator/(const long &val) const {
    return Rational{numerator, denominator * val};
}

Rational Rational::operator/(const Rational &val) const {
    throwIfDenomIsZero();

    long n = numerator*val.denominator;
    long d = denominator*val.numerator;
    return Rational{n, d};
}

Rational Rational::operator+(const long &val) const {
    long n = numerator + denominator*val;
    return Rational{n, denominator};
}

Rational Rational::operator+(const Rational &val) const {
    throwIfDenomIsZero();

    long n,d;
    if(denominator!=val.denominator) {
        n = numerator*val.denominator + val.numerator*denominator;
        d = denominator * val.denominator;
    } else {
        n = numerator + val.numerator;
        d = denominator;
    }
    return Rational{n, d};
}

Rational Rational::operator-(const long &val) const {
    long n = numerator - denominator*val;
    return Rational{n, denominator};
}

Rational Rational::operator-() const {
    return Rational{0, 1} - *this;
}

Rational Rational::operator-(const Rational &val) const {
    throwIfDenomIsZero();
    long n,d;
    if(denominator!=val.denominator) {
        n = numerator*val.denominator - val.numerator*denominator;
        d = denominator * val.denominator;
    } else {
        n = numerator - val.numerator;
        d = denominator;
    }

    return Rational{n, d};
}

void Rational::throwIfDenomIsZero() const {
    if(denominator==0)
        throw std::invalid_argument( "denom==0" );
}

bool Rational::operator!=(const long &rhs) const {
    return !(*this==rhs);
}

bool Rational::operator==(const long &rhs) const {
    if(!isRegular())
        return false;

    if(rhs!=0)
        return numerator==rhs*denominator;

    return numerator==0;
}

bool Rational::operator==(const Rational &rhs) const {
    if(!isRegular()) {
        if(isPositiveInfinity()==rhs.isPositiveInfinity())
            return true;
        else if(isNegativeInfinity()==rhs.isNegativeInfinity())
            return true;
    }

    return numerator*rhs.denominator==rhs.numerator*denominator;
}

bool Rational::operator!=(const Rational &val) const {
    return !(*this==val);
}

bool Rational::operator<=(const Rational &rhs) const {
    return (*this<rhs) || (*this==rhs);
}

bool Rational::operator>=(const Rational &rhs) {
    return  !(*this<rhs);
}

bool Rational::operator<(const Rational &rhs) const {
    if(!isRegular()) {
        if(isPositiveInfinity())
            return false;
        else return rhs.isPositiveInfinity();
    }

    makeDenominatorPositive();
    if(rhs != 0)
        return numerator*rhs.denominator -
               denominator*rhs.numerator < 0;
    else
        return isNegative();
}

bool Rational::operator>(const Rational &rhs) const {
    return  !(*this<=rhs);
}

bool Rational::isPositive() const {
    throwIfDenomIsZero();
    return (numerator>=0 && denominator>=0)||
           (numerator<=0 && denominator<=0);
}

bool Rational::isNegative() const {
    return !isPositive();
}

