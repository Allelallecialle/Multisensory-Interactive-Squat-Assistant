#pragma once

#include <cstddef>

class IIRFilter {
public:
    template <size_t Nb, size_t Na>
    IIRFilter(const double (&)[Nb], const double (&)[Na]) {}

    template <size_t Nb, size_t Na>
    IIRFilter(double (&)[Nb], double (&)[Na]) {}

    IIRFilter(const double*, const double*) {}

    double filter(double x) { return x; }
};
