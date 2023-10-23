#ifndef RAND_UTIL_H
#define RAND_UTIL_H

#include <random>

/* Generates an uniformly-random `double` in the range [`min`, `max`]
(by default [0, 1]). */
auto rand_double(double min = 0, double max = 1) {
    static std::mt19937 generator{std::random_device{}()};
    return std::uniform_real_distribution<>{min, max}(generator);
}

#endif