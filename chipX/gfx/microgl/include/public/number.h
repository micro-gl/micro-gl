#pragma once

template<typename T>
class number {
    T _number;

public:
    T & operator()() {
        return _number;
    };
};

class number_float: public number<float> {
};

template <unsigned P>
class number_q: public number<Q<P>> {
};
