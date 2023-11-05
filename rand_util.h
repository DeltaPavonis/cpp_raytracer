#ifndef RAND_UTIL_H
#define RAND_UTIL_H

#include <random>

/* Generates an uniformly-random `double` in the range [`min`, `max`]
(by default [0, 1]). Now trades accuracy for speed; we no longer use `<random>`
in favor of a very simple Lehmer PRNG. */
auto rand_double(double min = 0, double max = 1) {
    thread_local uint64_t seed = std::random_device{}();
    constexpr uint64_t MOD = 1ull << 32;
    constexpr double MOD_DB = 1 / static_cast<double>(MOD);
    seed = (seed * 1664525 + 1013904223) % MOD;
    return min + (max - min) * static_cast<double>(seed) * MOD_DB;
}

/* Generates an uniformly-random `int` in the range [`min`, `max`]
(by default [0, 1]). */
auto rand_int(int min = 0, int max = 1) {
    thread_local std::mt19937 generator{std::random_device{}()};
    thread_local std::uniform_int_distribution<> dist;
    dist.param(std::uniform_int_distribution<>::param_type{min, max});
    return dist(generator);
}

#endif