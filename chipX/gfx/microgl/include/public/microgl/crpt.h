#pragma once

template <typename T>
class crpt
{
protected:
    T& derived() { return static_cast<T&>(*this); }
    T const& derived() const { return static_cast<T const&>(*this); }
};

