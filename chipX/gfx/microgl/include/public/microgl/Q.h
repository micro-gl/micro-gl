//
// Created by Tomer Shalev on 2019-07-17.
//

#pragma once

#include <cstdint>


// WIP
class Q {
public:
    Q(const Q &q) {
        this->_bits = q.bits();
        this->_value = q.value();
    }

    Q(const Q &q, uint8_t $bits) {
        this->_value = q.value();

        scale($bits);
    }

    Q(const int val, uint8_t $bits) {

        this->_bits = $bits;
        this->_value = val<<$bits;
    }

    Q(const float val, uint8_t $bits) {

        this->_bits = $bits;
        this->_value = long(val * (1 << $bits));
    }

    // with assignments
    Q& operator =(const Q &q) {
        this->_bits = q.bits();
        this->_value = q.value();
        return *this;
    }

    Q& operator *=(const Q &q) {
        this->_value = ((long)this->_value*q.value());
        auto pre_bits = this->bits();
        this->_bits += q.bits();
        scale(pre_bits);
        return *this;
    }

    Q& operator /=(const Q &q) {
        // first scale up
        scaleUp(this->bits() + q.bits());

        this->_value /= q.value();
        this->_bits -= q.bits();

        return *this;
    }

    Q& operator +=(const Q &q) {
        this->_value += q.value();
        return *this;
    }

    Q& operator -=(const Q &q) {
        this->_value -= q.value();
        return *this;
    }

    // integer assignments
    template <typename T>
    Q& operator *=(const T i) {
        this->_value *= i;
        return *this;
    }

    template <typename T>
    Q& operator /=(const T i) {
        this->_value /= i;
        return *this;
    }

    template <typename T>
    Q& operator +=(const T i) {
        this->_value += i;
        return *this;
    }

    template <typename T>
    Q& operator -=(const T i) {
        this->_value -= i;
        return *this;
    }

    // intermidiate
    Q operator +(const Q &q) {
        Q temp(*this);
        temp += q;
        return temp;
    }

    Q operator *(const Q &q) {
        Q temp(*this);
        temp *= q;
        return temp;
    }

    Q operator /(const Q &q) {
        Q temp(*this);
        temp /= q;
        return temp;
    }

    Q operator -(const Q &q) {
        Q temp(*this);
        temp -= q;
        return temp;
    }

    // non Q
    template <typename T>
    Q operator +(const T i) {
        Q temp(*this);
        temp += i;
        return temp;
    }

    template <typename T>
    Q operator *(const T i) {
        Q temp(*this);
        temp *= i;
        return temp;
    }

    template <typename T>
    Q operator /(const T i) {
        Q temp(*this);
        temp /= i;
        return temp;
    }

    template <typename T>
    Q operator -(const T i) {
        Q temp(*this);
        temp -= i;
        return temp;
    }

    // convert
    int toInt() {
        return this->_value>>(this->_bits);
    }

    float toFloat() {
        return this->_value/float(1<<this->_bits);
    }

    void scale(uint8_t $bits) {
        if(_bits>$bits)
            scaleDown($bits);
        else
            scaleUp($bits);
    }

    void scaleUp(uint8_t $bits) {
        this->_bits = $bits;
        this->_value = (this->_value)<<($bits - this->_bits);
    }

    void scaleDown(uint8_t $bits) {
        this->_bits = $bits;
        this->_value = (this->_value)>>(this->_bits - $bits);
    }

    long fraction() {
        return _value & ((1<<_bits) - 1);
    }

    long integral() {
        return _value & (((1<<_bits) - 1)<<_bits);
    }

    inline uint8_t bits() const {
        return _bits;
    }

    inline long value() const {
        return _value;
    }

private:
    uint8_t _bits = 16;
    long _value = 0;
//    long _inter = 0;
};