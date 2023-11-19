#ifndef RAND_UTIL_H
#define RAND_UTIL_H

#include <random>
#include <optional>

/* `SeedSeqGenerator` generates seeds for RNG's. */
class SeedSeqGenerator {
    /* `rd` = a `std::random_device`, which will be the RNG seed generator used by default if
    no custom seed is provided. */
    std::random_device rd;
    /* `custom_seed` = a starting `uint64_t` used to generate a sequences of seeds with a LCG
    (Linear Congruential Generator). If unset, which is the default, then `rd` will be used. */
    std::optional<uint64_t> custom_seed;

public:

    /* Generates and returns the next random seed. */
    uint64_t operator() () {
        if (custom_seed) {  /* If custom seed provided, use it in a LCG */
            custom_seed = (*custom_seed * 483475) % 692253888527ULL;
            return *custom_seed;
        } else {
            /* Otherwise, just use the `std::random_device` to generate the next seed */
            return rd();
        }
    }

    /* Seeds the seed generator with `seed`. */
    void seed_with(uint64_t seed) {
        custom_seed = seed;
    }
} rng_seeds;

/* Generates an uniformly-random `double` in the range [`min`, `max`]
(by default [0, 1]). Now trades quality for speed; we no longer use `<random>`
in favor of a very simple Lehmer PRNG. */
auto rand_double(double min = 0, double max = 1) {
    thread_local uint64_t seed = rng_seeds();  /* Seed with the next seed */
    constexpr uint64_t MOD = 1ull << 32;
    constexpr double MOD_DB = 1 / static_cast<double>(MOD);
    seed = (seed * 1664525 + 1013904223) % MOD;
    return min + (max - min) * static_cast<double>(seed) * MOD_DB;
}

/* Generates an uniformly-random `int` in the range [`min`, `max`]
(by default [0, 1]). */
auto rand_int(int min = 0, int max = 1) {
    thread_local std::mt19937 generator{rng_seeds()};  /* Seed with the next seed */
    thread_local std::uniform_int_distribution<> dist;
    dist.param(std::uniform_int_distribution<>::param_type{min, max});
    return dist(generator);
}

#endif